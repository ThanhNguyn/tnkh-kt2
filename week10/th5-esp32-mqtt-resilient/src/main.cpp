#include <Arduino.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <cmath>
#include <ctype.h>
#include <esp_system.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "ring_buffer.h"

namespace
{

  struct Sample
  {
    uint32_t seq = 0;
    uint32_t ts = 0;
    float temperature = 0;
    float humidity = 0;
    bool buffered = false;
  };

  WiFiClient netClient;
  PubSubClient mqtt(netClient);
  WebServer http(HTTP_PORT);
  DHT dht(PIN_DHT, DHT22);
  RingBuffer<Sample, BUFFER_CAPACITY> backlog;

  char clientId[24];
  uint32_t bootId = 0;
  uint32_t seqCounter = 0;
  Sample lastSample;
  bool haveSample = false;
  uint32_t sensorErrors = 0;

  bool ledOn = false;
  bool outageSim = false;
  bool httpStarted = false;

  uint32_t mqttDelay = MQTT_BACKOFF_MIN_MS;
  uint32_t nextMqttAttempt = 0;
  bool mqttWasUp = false;
  bool mqttLost = false;
  uint32_t mqttLostAt = 0;
  uint32_t reconnectCount = 0;
  uint32_t publishFailures = 0;
  uint32_t lastSampleAt = 0;
  uint32_t lastFlushAt = 0;

  uint32_t epochNow()
  {
    time_t t = time(nullptr);
    return t > 1700000000 ? static_cast<uint32_t>(t) : 0; // 0 = clock not synced
  }

  void setLed(bool on, bool announce)
  {
    ledOn = on;
    digitalWrite(PIN_LED, on ? HIGH : LOW);
    if (announce && mqtt.connected())
    {
      mqtt.publish(TOPIC_LED_STATE, on ? "ON" : "OFF", true);
    }
  }

  void printStatus()
  {
    Serial.printf(
        "[status] uptime=%lus wifi=%s mqtt=%s buffered=%u dropped=%lu "
        "reconnects=%lu sensorErrors=%lu outageSim=%s led=%s\n",
        static_cast<unsigned long>(millis() / 1000),
        WiFi.status() == WL_CONNECTED ? "up" : "down",
        mqtt.connected() ? "up" : "down", static_cast<unsigned>(backlog.size()),
        static_cast<unsigned long>(backlog.dropped()),
        static_cast<unsigned long>(reconnectCount),
        static_cast<unsigned long>(sensorErrors), outageSim ? "ON" : "off",
        ledOn ? "ON" : "OFF");
  }

  void onMessage(char *topic, uint8_t *payload, unsigned int len)
  {
    if (strcmp(topic, TOPIC_LED_CMD) != 0)
      return;
    char buf[8];
    size_t n = len < sizeof(buf) - 1 ? len : sizeof(buf) - 1;
    memcpy(buf, payload, n);
    buf[n] = '\0';
    for (size_t i = 0; i < n; ++i)
      buf[i] = static_cast<char>(toupper(buf[i]));

    if (!strcmp(buf, "ON") || !strcmp(buf, "1") || !strcmp(buf, "TRUE"))
    {
      setLed(true, true);
    }
    else if (!strcmp(buf, "OFF") || !strcmp(buf, "0") || !strcmp(buf, "FALSE"))
    {
      setLed(false, true);
    }
    else
    {
      Serial.printf("[led] ignored unknown command '%s'\n", buf);
    }
  }

  bool publishSample(const Sample &s)
  {
    char buf[160];
    int n = snprintf(buf, sizeof(buf),
                     "{\"boot\":%lu,\"seq\":%lu,\"ts\":%lu,\"t\":%.1f,\"h\":%.1f,"
                     "\"replayed\":%s}",
                     static_cast<unsigned long>(bootId),
                     static_cast<unsigned long>(s.seq),
                     static_cast<unsigned long>(s.ts), s.temperature, s.humidity,
                     s.buffered ? "true" : "false");
    if (n <= 0 || n >= static_cast<int>(sizeof(buf)))
      return false;
    return mqtt.publish(TOPIC_SENSOR, buf, false);
  }

  void announceOnline()
  {
    char info[160];
    snprintf(info, sizeof(info),
             "{\"fw\":\"%s\",\"boot\":%lu,\"ip\":\"%s\",\"reconnects\":%lu}",
             FW_VERSION, static_cast<unsigned long>(bootId),
             WiFi.localIP().toString().c_str(),
             static_cast<unsigned long>(reconnectCount));
    mqtt.publish(TOPIC_STATUS, "online", true);
    mqtt.publish(TOPIC_INFO, info, true);
    mqtt.publish(TOPIC_LED_STATE, ledOn ? "ON" : "OFF", true);
    mqtt.subscribe(TOPIC_LED_CMD);
  }

  void maintainMqtt()
  {
    bool up = mqtt.connected();
    if (up != mqttWasUp)
    {
      mqttWasUp = up;
      if (!up)
      {
        mqttLost = true;
        mqttLostAt = millis();
        Serial.println("[mqtt] connection lost");
      }
    }
    if (up || outageSim || WiFi.status() != WL_CONNECTED)
      return;

    uint32_t now = millis();
    if (static_cast<int32_t>(now - nextMqttAttempt) < 0)
      return;

    Serial.printf("[mqtt] connecting to %s:%u as %s\n", MQTT_HOST, MQTT_PORT,
                  clientId);
    if (mqtt.connect(clientId, nullptr, nullptr, TOPIC_STATUS, 1, true, "offline"))
    {
      if (mqttLost)
      {
        ++reconnectCount;
        Serial.printf("[mqtt] reconnected after %lu ms, %u samples buffered\n",
                      static_cast<unsigned long>(millis() - mqttLostAt),
                      static_cast<unsigned>(backlog.size()));
        mqttLost = false;
      }
      else
      {
        Serial.println("[mqtt] connected");
      }
      mqttWasUp = true;
      mqttDelay = MQTT_BACKOFF_MIN_MS;
      announceOnline();
    }
    else
    {
      uint32_t wait = mqttDelay + static_cast<uint32_t>(random(0, MQTT_JITTER_MS));
      Serial.printf("[mqtt] connect failed (state=%d), retry in %lu ms\n",
                    mqtt.state(), static_cast<unsigned long>(wait));
      nextMqttAttempt = now + wait;
      mqttDelay = (mqttDelay * 2 > MQTT_BACKOFF_MAX_MS) ? MQTT_BACKOFF_MAX_MS
                                                        : mqttDelay * 2;
    }
  }

  void maintainWifi()
  {
    static wl_status_t last = WL_NO_SHIELD;
    static uint32_t downSince = 0;
    if (outageSim)
      return;

    wl_status_t st = WiFi.status();
    if (st != last)
    {
      last = st;
      if (st == WL_CONNECTED)
      {
        Serial.printf("[wifi] connected, IP %s\n",
                      WiFi.localIP().toString().c_str());
        if (!httpStarted)
        {
          http.begin();
          httpStarted = true;
          Serial.printf("[http] listening on port %u\n", HTTP_PORT);
        }
      }
      else
      {
        downSince = millis();
        Serial.printf("[wifi] not connected (status %d)\n", static_cast<int>(st));
      }
    }
    if (st != WL_CONNECTED && millis() - downSince > WIFI_RETRY_MS)
    {
      downSince = millis();
      Serial.println("[wifi] retrying");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
  }

  void setOutage(bool on)
  {
    if (on == outageSim)
      return;
    outageSim = on;
    if (on)
    {
      Serial.println("[sim] network outage START");
      netClient.stop();
      WiFi.disconnect(false, false);
    }
    else
    {
      Serial.println("[sim] network outage END");
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      mqttDelay = MQTT_BACKOFF_MIN_MS;
      nextMqttAttempt = 0;
    }
  }
  void sampleTick()
  {
    uint32_t now = millis();
    if (now - lastSampleAt < SAMPLE_INTERVAL_MS)
      return;
    lastSampleAt = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (std::isnan(t) || std::isnan(h))
    {
      ++sensorErrors;
      Serial.println("[dht] read failed, sample skipped");
      return;
    }

    Sample s;
    s.seq = ++seqCounter;
    s.ts = epochNow();
    s.temperature = t;
    s.humidity = h;
    lastSample = s;
    haveSample = true;
    if (mqtt.connected() && backlog.empty() && publishSample(s))
    {
      Serial.printf("[pub] seq=%lu t=%.1f h=%.1f\n",
                    static_cast<unsigned long>(s.seq), t, h);
      return;
    }
    s.buffered = true;
    backlog.push(s);
    Serial.printf("[buf] seq=%lu t=%.1f h=%.1f (queued=%u)\n",
                  static_cast<unsigned long>(s.seq), t, h,
                  static_cast<unsigned>(backlog.size()));
  }

  void flushBacklog()
  {
    if (!mqtt.connected() || backlog.empty())
      return;
    uint32_t now = millis();
    if (now - lastFlushAt < FLUSH_INTERVAL_MS)
      return;
    lastFlushAt = now;

    if (publishSample(backlog.front()))
    {
      backlog.pop();
      publishFailures = 0;
    }
    else if (++publishFailures >= 3)
    {
      Serial.println("[buf] dropping a sample that repeatedly fails to publish");
      backlog.pop();
      publishFailures = 0;
    }
  }
  void handleButton()
  {
    static int lastRaw = HIGH;
    static int stable = HIGH;
    static uint32_t changedAt = 0;

    int raw = digitalRead(PIN_BTN);
    if (raw != lastRaw)
    {
      lastRaw = raw;
      changedAt = millis();
    }
    if (millis() - changedAt >= 50 && raw != stable)
    {
      stable = raw;
      if (stable == LOW)
        setOutage(!outageSim);
    }
  }

  void handleSerial()
  {
    while (Serial.available())
    {
      int c = Serial.read();
      if (c == 'o' || c == 'O')
      {
        setOutage(!outageSim);
      }
      else if (c == 's' || c == 'S')
      {
        printStatus();
      }
    }
  }
  void handleSensor()
  {
    if (!haveSample)
    {
      http.send(503, "application/json", "{\"error\":\"no sample yet\"}");
      return;
    }
    char buf[200];
    snprintf(buf, sizeof(buf),
             "{\"temperature\":%.1f,\"humidity\":%.1f,\"seq\":%lu,\"ts\":%lu,"
             "\"mqtt\":\"%s\",\"buffered\":%u}",
             lastSample.temperature, lastSample.humidity,
             static_cast<unsigned long>(lastSample.seq),
             static_cast<unsigned long>(lastSample.ts),
             mqtt.connected() ? "connected" : "disconnected",
             static_cast<unsigned>(backlog.size()));
    http.send(200, "application/json", buf);
  }

  void handleStatus()
  {
    char buf[320];
    snprintf(buf, sizeof(buf),
             "{\"fw\":\"%s\",\"uptime_s\":%lu,\"wifi\":\"%s\",\"rssi\":%d,"
             "\"mqtt\":\"%s\",\"buffered\":%u,\"dropped\":%lu,\"reconnects\":%lu,"
             "\"sensor_errors\":%lu,\"outage_sim\":%s,\"led\":\"%s\"}",
             FW_VERSION, static_cast<unsigned long>(millis() / 1000),
             WiFi.status() == WL_CONNECTED ? "connected" : "disconnected",
             static_cast<int>(WiFi.RSSI()),
             mqtt.connected() ? "connected" : "disconnected",
             static_cast<unsigned>(backlog.size()),
             static_cast<unsigned long>(backlog.dropped()),
             static_cast<unsigned long>(reconnectCount),
             static_cast<unsigned long>(sensorErrors), outageSim ? "true" : "false",
             ledOn ? "ON" : "OFF");
    http.send(200, "application/json", buf);
  }

}
void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nTH5 resilient MQTT node, firmware %s\n", FW_VERSION);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  setLed(false, false);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  bootId = esp_random();
  snprintf(clientId, sizeof(clientId), "th5-%08lx",
           static_cast<unsigned long>(bootId));

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(MQTT_KEEPALIVE_S);
  mqtt.setSocketTimeout(MQTT_SOCKET_TIMEOUT_S);

  http.on("/api/sensor", HTTP_GET, handleSensor);
  http.on("/api/status", HTTP_GET, handleStatus);
  http.onNotFound([]()
                  { http.send(404, "application/json", "{\"error\":\"not found\"}"); });

  Serial.println("Press the button (GPIO15) or send 'o' to toggle a simulated "
                 "outage, 's' for status.");
}

void loop()
{
  handleButton();
  handleSerial();
  maintainWifi();
  maintainMqtt();
  if (mqtt.connected())
    mqtt.loop();
  if (httpStarted)
    http.handleClient();
  sampleTick();
  flushBacklog();
  delay(2);
}
