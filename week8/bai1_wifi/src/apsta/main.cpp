#include <Arduino.h>
#include <WiFi.h>

#include "wifi_config.h"

namespace {
constexpr uint32_t STATUS_INTERVAL_MS = 2000;
uint32_t lastStatusAt = 0;
bool staConnectionReported = false;

void printStaConnectionInfo() {
    Serial.println("[STA] Connected to emulator access point");
    Serial.printf("[STA] SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("[STA] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[STA] Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("[STA] RSSI: %d dBm\n", WiFi.RSSI());
    Serial.printf("[STA] BSSID: %s\n", WiFi.BSSIDstr().c_str());
}
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("TH3 - BAI 1 - WIFI AP + STA (APSTA)");
    Serial.println("========================================");

    WiFi.mode(WIFI_AP_STA);

    // Keep the firmware AP subnet separate from the emulator's upstream AP.
    const IPAddress apAddress(192, 168, 5, 1);
    const IPAddress apGateway(192, 168, 5, 1);
    const IPAddress apSubnet(255, 255, 255, 0);
    if (!WiFi.softAPConfig(apAddress, apGateway, apSubnet)) {
        Serial.println("[ERROR] SoftAP network configuration failed");
        while (true) {
            delay(1000);
        }
    }

    const bool apStarted = WiFi.softAP(
        WIFI_SSID,
        WIFI_PASSWORD,
        WIFI_AP_CHANNEL,
        false,
        WIFI_AP_MAX_STATIONS
    );

    if (!apStarted) {
        Serial.println("[ERROR] SoftAP initialization failed");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("[OK] SoftAP started");
    Serial.printf("[AP] SSID: %s\n", WIFI_SSID);
    Serial.printf("[AP] IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("[AP] MAC: %s\n", WiFi.softAPmacAddress().c_str());

    WiFi.setAutoReconnect(true);
    Serial.printf("[STA] Connecting to emulator AP: %s\n", WIFI_EMU_UPLINK_SSID);
    WiFi.begin(WIFI_EMU_UPLINK_SSID, WIFI_EMU_UPLINK_PASSWORD);

    lastStatusAt = millis();
}

void loop() {
    const bool staConnected = WiFi.status() == WL_CONNECTED;

    if (staConnected && !staConnectionReported) {
        staConnectionReported = true;
        printStaConnectionInfo();
    } else if (!staConnected && staConnectionReported) {
        staConnectionReported = false;
        Serial.println("[STA] Connection lost; reconnecting");
    }

    const uint32_t now = millis();
    if (now - lastStatusAt >= STATUS_INTERVAL_MS) {
        lastStatusAt = now;
        Serial.printf("[AP] Connected stations: %u\n", WiFi.softAPgetStationNum());
        Serial.printf("[STA] Status: %s\n", staConnected ? "CONNECTED" : "CONNECTING");
    }

    delay(50);
}
