#pragma once
#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Network
// ---------------------------------------------------------------------------
#define WIFI_SSID "Wokwi-GUEST"   // open network provided by the Wokwi simulator
#define WIFI_PASS ""
#define MQTT_HOST "broker.hivemq.com"
constexpr uint16_t MQTT_PORT = 1883;

// The public HiveMQ broker is shared by everyone. Make TOPIC_ROOT unique
// (for example append your student ID) and use the same value in the
// Node-RED flow, otherwise you will see other people's data.
#define TOPIC_ROOT "tnkh/th5/esp32"
#define TOPIC_SENSOR TOPIC_ROOT "/sensor"        // device -> broker, JSON samples
#define TOPIC_STATUS TOPIC_ROOT "/status"        // retained: "online" / "offline" (LWT)
#define TOPIC_INFO TOPIC_ROOT "/info"            // retained: firmware / boot info
#define TOPIC_LED_CMD TOPIC_ROOT "/led/set"      // broker -> device: "ON" / "OFF"
#define TOPIC_LED_STATE TOPIC_ROOT "/led/state"  // retained: current LED state

// ---------------------------------------------------------------------------
// Hardware
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_DHT = 4;   // DHT22 data
constexpr uint8_t PIN_LED = 2;   // LED (through a resistor)
constexpr uint8_t PIN_BTN = 15;  // push button to GND, toggles simulated outage

// ---------------------------------------------------------------------------
// Timing and resilience
// ---------------------------------------------------------------------------
constexpr uint32_t SAMPLE_INTERVAL_MS = 2000;   // sensor period
constexpr uint32_t FLUSH_INTERVAL_MS = 150;     // pace of buffered replay
constexpr size_t BUFFER_CAPACITY = 120;         // ~4 min of samples at 2 s
constexpr uint32_t MQTT_BACKOFF_MIN_MS = 1000;
constexpr uint32_t MQTT_BACKOFF_MAX_MS = 30000;
constexpr uint32_t MQTT_JITTER_MS = 500;
constexpr uint16_t MQTT_KEEPALIVE_S = 15;       // broker notices a dead device in ~22 s
constexpr uint16_t MQTT_SOCKET_TIMEOUT_S = 3;   // keeps loop() responsive while offline
constexpr uint32_t WIFI_RETRY_MS = 15000;
constexpr uint16_t HTTP_PORT = 80;

#define FW_VERSION "1.0.0"
