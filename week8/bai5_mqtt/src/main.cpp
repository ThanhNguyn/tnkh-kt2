#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <cstring>

#define LED_PIN 2

#if defined(WOKWI_SIMULATION)
const char *ssid = "Wokwi-GUEST";
const char *password = "";
#else
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
#endif

const char *broker = "broker.hivemq.com";
const uint16_t brokerPort = 1883;

const char *counterTopic = "bcse/th3/nhom-02";
const char *ledTopic = "bcse/th3/nhom-02/led";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

char clientId[32];
uint32_t counter = 0;
uint32_t lastPublish = 0;
uint32_t lastWiFiRetry = 0;
uint32_t lastMqttRetry = 0;
bool wifiWasConnected = false;
bool ledOn = false;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("[MQTT RX] Topic: %s\n", topic);
  Serial.printf("[MQTT RX] Length: %u\n", length);
  Serial.print("[MQTT RX] Payload: ");

  for (unsigned int i = 0; i < length; i++)
  {
    Serial.write(payload[i]);
  }
  Serial.println();

  if (strcmp(topic, ledTopic) != 0)
  {
    Serial.println("[MQTT RX] Wrong topic");
    return;
  }

  if (length == 1 && payload[0] == '1')
  {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
  }
  else if (length == 1 && payload[0] == '0')
  {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
  }
  else
  {
    Serial.println("[MQTT RX] Invalid payload");
  }
}

void beginWiFi()
{
  WiFi.mode(WIFI_STA);
#if defined(WOKWI_SIMULATION)
  WiFi.begin(ssid, password, 6);
#else
  WiFi.begin(ssid, password);
#endif
  lastWiFiRetry = millis();
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
}

void maintainWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!wifiWasConnected)
    {
      wifiWasConnected = true;
      Serial.println("Wi-Fi connected");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
    return;
  }

  wifiWasConnected = false;

  if (millis() - lastWiFiRetry >= 5000)
  {
    lastWiFiRetry = millis();
    WiFi.reconnect();
  }
}

void makeClientId()
{
  uint64_t chipId = ESP.getEfuseMac();
  snprintf(
      clientId,
      sizeof(clientId),
      "esp32-th3-%04X%08X",
      static_cast<uint16_t>(chipId >> 32),
      static_cast<uint32_t>(chipId));
}

void connectMqtt()
{
  if (mqtt.connected() || millis() - lastMqttRetry < 3000)
  {
    return;
  }

  lastMqttRetry = millis();

  Serial.printf(
      "[MQTT] Connecting to %s:%u...\n",
      broker,
      brokerPort);

  IPAddress brokerIP;

  if (WiFi.hostByName(broker, brokerIP))
  {
    Serial.printf("[DNS] Broker IP: %s\n", brokerIP.toString().c_str());
  }
  else
  {
    Serial.println("[DNS] Broker hostname resolution failed");
  }

  if (mqtt.connect(clientId))
  {
    Serial.println("[MQTT] Connected!");

    const bool subscribed = mqtt.subscribe(ledTopic);

    Serial.printf(
        "[MQTT] Subscribe %s: %s\n",
        ledTopic,
        subscribed ? "OK" : "FAILED");

    Serial.printf("[MQTT] Client ID: %s\n", clientId);
  }
  else
  {
    Serial.printf(
        "[MQTT] Connection failed, state = %d\n",
        mqtt.state());
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  makeClientId();
  mqtt.setServer(broker, brokerPort);
  mqtt.setCallback(mqttCallback);
  mqtt.setKeepAlive(30);
  mqtt.setBufferSize(256);

  beginWiFi();
}

void loop()
{
  maintainWiFi();

  if (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    return;
  }

  connectMqtt();
  mqtt.loop();

  if (mqtt.connected() && millis() - lastPublish >= 2000)
  {
    lastPublish = millis();
    String value = String(counter++);

    if (mqtt.publish(counterTopic, value.c_str()))
    {
      Serial.print("Published ");
      Serial.print(value);
      Serial.print(" to ");
      Serial.println(counterTopic);
    }
  }

  delay(1);
}
