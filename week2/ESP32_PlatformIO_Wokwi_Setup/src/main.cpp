#include <Arduino.h>

constexpr uint8_t LED_PIN = 2;

bool ledState = false;
uint32_t lastToggle = 0;

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println();
    Serial.println("=================================");
    Serial.println("CHUAN BI 3 - ESP32 + WOKWI");
    Serial.println("Board: ESP32 DevKit v1");
    Serial.println("PlatformIO: esp32dev");
    Serial.println("Serial Monitor: 115200");
    Serial.println("Simulation started");
    Serial.println("=================================");
}

void loop() {
    const uint32_t now = millis();

    if (now - lastToggle >= 500) {
        lastToggle = now;

        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);

        Serial.printf(
            "t=%lu ms | LED=%s\n",
            now,
            ledState ? "ON" : "OFF"
        );
    }
}