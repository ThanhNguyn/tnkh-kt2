#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "project_config.h"

DHT dht(DHT_PIN, DHT_SENSOR_TYPE);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

char telemetryTopic[100];
char clientId[48];

unsigned long lastPublishAt = 0;
unsigned long lastMqttAttemptAt = 0;
unsigned long lastWiFiAttemptAt = 0;

void connectWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.printf("Connecting to Wi-Fi: %s", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void prepareTopicsAndClientId()
{
    snprintf(telemetryTopic, sizeof(telemetryTopic),
             "aet2022/%s/telemetry", SV_ID);

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    snprintf(clientId, sizeof(clientId), "aet2022-%s", mac.c_str());

    Serial.printf("MQTT client ID: %s\n", clientId);
    Serial.printf("Publish topic: %s\n", telemetryTopic);
}

void maintainWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    const unsigned long now = millis();
    if (now - lastWiFiAttemptAt >= WIFI_RETRY_INTERVAL_MS)
    {
        lastWiFiAttemptAt = now;
        Serial.println("Wi-Fi disconnected; reconnecting...");
        WiFi.reconnect();
    }
}

void maintainMqtt()
{
    if (WiFi.status() != WL_CONNECTED || mqtt.connected())
        return;

    const unsigned long now = millis();
    if (now - lastMqttAttemptAt < MQTT_RETRY_INTERVAL_MS)
        return;
    lastMqttAttemptAt = now;

    Serial.printf("Connecting to MQTT %s:%d ...\n", MQTT_HOST, MQTT_PORT);
    if (mqtt.connect(clientId))
    {
        Serial.println("MQTT connected.");
    }
    else
    {
        Serial.printf("MQTT connection failed, state=%d\n", mqtt.state());
    }
}

void publishTelemetry()
{
    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("DHT read failed; telemetry skipped.");
        return;
    }

    JsonDocument doc;
    doc["id"] = SV_ID;
    doc["temp"] = temperature;
    doc["hum"] = humidity;
    doc["uptime"] = millis() / 1000UL;
    doc["rssi"] = WiFi.RSSI();

    char payload[256];
    const size_t length = serializeJson(doc, payload, sizeof(payload));
    if (length == 0)
    {
        Serial.println("JSON serialization failed.");
        return;
    }

    if (mqtt.publish(telemetryTopic, payload))
    {
        Serial.printf("Published %s -> %s\n", telemetryTopic, payload);
    }
    else
    {
        Serial.println("MQTT publish failed.");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(300);
    dht.begin();

    Serial.println("\n==================================");
    Serial.println("AET2022 TH4 - BAI 2 - MQTT");
    Serial.println("==================================");

    connectWiFi();
    prepareTopicsAndClientId();

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setBufferSize(512);
    mqtt.setKeepAlive(30);
}

void loop()
{
    maintainWiFi();

    if (WiFi.status() == WL_CONNECTED)
    {
        if (!mqtt.connected())
            maintainMqtt();
        mqtt.loop();
    }

    const unsigned long now = millis();
    if (mqtt.connected() && now - lastPublishAt >= TELEMETRY_INTERVAL_MS)
    {
        lastPublishAt = now;
        publishTelemetry();
    }

    delay(2);
}
