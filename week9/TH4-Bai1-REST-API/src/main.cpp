#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include "project_config.h"

DHT dht(DHT_PIN, DHT_SENSOR_TYPE);
WebServer server(80);

void handleSensor() {
    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();

    JsonDocument doc;
    doc["id"] = SV_ID;
    doc["rssi"] = WiFi.isConnected() ? WiFi.RSSI() : 0;
    doc["uptime"] = millis() / 1000UL;
    doc["sensor_ok"] = !isnan(temperature) && !isnan(humidity);

    if (isnan(temperature) || isnan(humidity)) {
        doc["error"] = "sensor_read_failed";
        String body;
        serializeJson(doc, body);
        server.send(503, "application/json; charset=utf-8", body);
        Serial.println("Sensor read failed; returned HTTP 503 JSON.");
        return;
    }

    doc["temp"] = temperature;
    doc["hum"] = humidity;

    String body;
    serializeJson(doc, body);
    server.send(200, "application/json; charset=utf-8", body);

    Serial.println("GET /api/sensor");
    Serial.println(body);
}

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.printf("Connecting to Wi-Fi: %s", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi connected.");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
}

void setup() {
    Serial.begin(115200);
    delay(300);
    dht.begin();

    Serial.println("\n==================================");
    Serial.println("AET2022 TH4 - BAI 1 - REST API");
    Serial.println("==================================");

    connectWiFi();

    server.on("/", HTTP_GET, []() {
        server.send(200, "text/plain; charset=utf-8",
                    "AET2022 TH4 REST server is running. Try GET /api/sensor");
    });

    server.on("/api/sensor", HTTP_GET, handleSensor);

    server.onNotFound([]() {
        JsonDocument doc;
        doc["error"] = "not_found";
        String body;
        serializeJson(doc, body);
        server.send(404, "application/json; charset=utf-8", body);
    });

    server.begin();
    Serial.println("HTTP server started on port 80.");
    Serial.println("Endpoint: GET /api/sensor");
    Serial.println("Wokwi host URL: http://localhost:8180/api/sensor");
}

void loop() {
    server.handleClient();
    delay(2);
}
