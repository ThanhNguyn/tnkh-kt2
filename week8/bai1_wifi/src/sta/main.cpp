#include <Arduino.h>
#include <WiFi.h>

#include "wifi_config.h"

namespace
{

#if defined(BREADBOARD_SIM)

    // Breadboard virtual Wi-Fi network
    constexpr char STA_SSID[] = "BreadboardNet";

#else

    // Wi-Fi AP used by the real ESP32 hardware
    constexpr char STA_SSID[] = WIFI_SSID;
    constexpr char STA_PASSWORD[] = WIFI_PASSWORD;

#endif

    constexpr uint32_t CONNECT_TIMEOUT_MS = 30000;

    uint32_t connectStartedAt = 0;

    bool connectionReported = false;
    bool timeoutReported = false;

    void scanNetworks()
    {
        Serial.println("[SCAN] Scanning Wi-Fi networks...");

        const int networkCount = WiFi.scanNetworks(false, true);

        Serial.printf(
            "[SCAN] Found %d networks\n",
            networkCount);

        for (int i = 0; i < networkCount; i++)
        {
            Serial.printf(
                "[SCAN] %s | RSSI: %d | Channel: %d\n",
                WiFi.SSID(i).c_str(),
                WiFi.RSSI(i),
                WiFi.channel(i));
        }

        WiFi.scanDelete();
    }

    void printConnectionInfo()
    {
        Serial.println("[OK] STA connected");

        Serial.printf(
            "SSID: %s\n",
            WiFi.SSID().c_str());

        Serial.printf(
            "STA MAC: %s\n",
            WiFi.macAddress().c_str());

        Serial.printf(
            "STA IP: %s\n",
            WiFi.localIP().toString().c_str());

        Serial.printf(
            "Gateway: %s\n",
            WiFi.gatewayIP().toString().c_str());

        Serial.printf(
            "Subnet: %s\n",
            WiFi.subnetMask().toString().c_str());

        Serial.printf(
            "RSSI: %d dBm\n",
            WiFi.RSSI());

        Serial.printf(
            "Channel: %d\n",
            WiFi.channel());

        Serial.println("----------------------------------------");
    }

    void connectToNetwork()
    {
#if defined(BREADBOARD_SIM)

        Serial.printf(
            "[INFO] Connecting to Breadboard network: %s\n",
            STA_SSID);

        WiFi.begin(STA_SSID);

#else

        Serial.printf(
            "[INFO] Connecting to SSID: %s\n",
            STA_SSID);

        WiFi.begin(STA_SSID, STA_PASSWORD);

#endif

        connectStartedAt = millis();
    }

}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("TH3 - BAI 1 - WIFI STATION (STA)");
    Serial.println("========================================");

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    Serial.printf(
        "STA MAC: %s\n",
        WiFi.macAddress().c_str());

    scanNetworks();

    connectToNetwork();
}

void loop()
{
    const bool connected = WiFi.status() == WL_CONNECTED;

    if (connected && !connectionReported)
    {
        connectionReported = true;
        timeoutReported = false;

        printConnectionInfo();
    }

    if (!connected && connectionReported)
    {
        connectionReported = false;

        Serial.println("[WARN] Wi-Fi connection lost");
    }

    if (
        !connected &&
        !timeoutReported &&
        millis() - connectStartedAt >= CONNECT_TIMEOUT_MS)
    {
        timeoutReported = true;

        Serial.printf(
            "[ERROR] Connection timed out. WiFi status: %d\n",
            static_cast<int>(WiFi.status()));
    }

    delay(100);
}