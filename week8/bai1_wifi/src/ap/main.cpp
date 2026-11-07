#include <Arduino.h>
#include <WiFi.h>

#include "wifi_config.h"

namespace
{
    constexpr uint32_t STATUS_INTERVAL_MS = 2000;
    uint32_t lastStatusAt = 0;
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("TH3 - BAI 1 - WIFI ACCESS POINT (AP)");
    Serial.println("========================================");

    WiFi.mode(WIFI_AP);

    const bool started = WiFi.softAP(
        WIFI_SSID,
        WIFI_PASSWORD,
        WIFI_AP_CHANNEL,
        false,
        WIFI_AP_MAX_STATIONS);

    if (!started)
    {
        Serial.println("[ERROR] SoftAP initialization failed");

        while (true)
        {
            delay(1000);
        }
    }

    delay(100);

    Serial.println("[OK] Access Point started");
    Serial.printf("SSID: %s\n", WIFI_SSID);
    Serial.printf("Password: %s\n", WIFI_PASSWORD);
    Serial.printf(
        "AP IP: %s\n",
        WiFi.softAPIP().toString().c_str());
    Serial.printf(
        "AP MAC: %s\n",
        WiFi.softAPmacAddress().c_str());
    Serial.printf("Channel: %u\n", WIFI_AP_CHANNEL);

    Serial.println("----------------------------------------");

    lastStatusAt = millis();
}

void loop()
{
    const uint32_t now = millis();

    if (now - lastStatusAt >= STATUS_INTERVAL_MS)
    {
        lastStatusAt = now;

        Serial.printf(
            "[AP] Connected stations: %u\n",
            WiFi.softAPgetStationNum());
    }

    delay(10);
}