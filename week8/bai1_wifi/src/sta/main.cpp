#include <Arduino.h>
#include <WiFi.h>

#include "wifi_config.h"

namespace {
uint32_t lastAttemptAt = 0;
bool connectionReported = false;

void printConnectionInfo() {
    Serial.println("[OK] STA connected");
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("STA IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("Subnet: %s\n", WiFi.subnetMask().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    Serial.printf("BSSID: %s\n", WiFi.BSSIDstr().c_str());
    Serial.println("----------------------------------------");
}

bool connectToConfiguredNetwork() {
    Serial.printf("[INFO] Connecting to SSID: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const uint32_t startedAt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        connectionReported = true;
        printConnectionInfo();
        return true;
    }

    connectionReported = false;
    lastAttemptAt = millis();
    Serial.println("[ERROR] Connection timed out");
    return false;
}
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("TH3 - BAI 1 - WIFI STATION (STA)");
    Serial.println("========================================");

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    connectToConfiguredNetwork();
}

void loop() {
    const bool connected = WiFi.status() == WL_CONNECTED;

    if (connected && !connectionReported) {
        connectionReported = true;
        printConnectionInfo();
    } else if (!connected && connectionReported) {
        connectionReported = false;
        lastAttemptAt = millis();
        Serial.println("[WARN] Wi-Fi connection lost");
    }

    if (!connected && millis() - lastAttemptAt >= WIFI_RETRY_INTERVAL_MS) {
        connectToConfiguredNetwork();
    }

    delay(100);
}
