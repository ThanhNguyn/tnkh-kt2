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

const char *broker = "test.mosquitto.org";
const uint16_t brokerPort = 1883;
const char *counterTopic = "bcse/th3/nhom-XX";
const char *ledTopic = "bcse/th3/nhom-XX/led";

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
  if (strcmp(topic, ledTopic) != 0 || length != 1)
  {
    return;
  }

  if (payload[0] == '1')
  {
    ledOn = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
  }
  else if (payload[0] == '0')
  {
    ledOn = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
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

  if (mqtt.connect(clientId))
  {
    mqtt.subscribe(ledTopic);
    Serial.println("MQTT connected");
    Serial.print("Client ID: ");
    Serial.println(clientId);
    Serial.print("Subscribed: ");
    Serial.println(ledTopic);
  }
  else
  {
    Serial.print("MQTT connection failed: ");
    Serial.println(mqtt.state());
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
