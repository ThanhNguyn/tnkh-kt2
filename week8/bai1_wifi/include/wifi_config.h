#pragma once

#define WIFI_SSID "MyESP32AP"
#define WIFI_PASSWORD "password123"

constexpr uint8_t WIFI_AP_CHANNEL = 6;
constexpr uint8_t WIFI_AP_MAX_STATIONS = 4;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 30000;
constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 5000;

#define WIFI_EMU_UPLINK_SSID "EMU_UPLINK"
#define WIFI_EMU_UPLINK_PASSWORD "EmuUplink123"