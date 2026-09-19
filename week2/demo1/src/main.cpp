#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t LED_PIN = 2;
constexpr uint32_t DEBOUNCE_MS = 50;

volatile uint32_t pressCount = 0;
volatile uint32_t lastInterruptTime = 0;
volatile bool buttonEvent = false;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
bool ledState = false;

void IRAM_ATTR buttonISR()
{
    const uint32_t now = millis();

    if (now - lastInterruptTime >= DEBOUNCE_MS)
    {
        lastInterruptTime = now;

        portENTER_CRITICAL_ISR(&mux);
        ++pressCount;
        buttonEvent = true;
        portEXIT_CRITICAL_ISR(&mux);
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        buttonISR,
        FALLING);

    Serial.println("=== DEMO 1 - EXTERNAL INTERRUPT ===");
    Serial.println("ESP32 ready");
}

void loop()
{
    bool event = false;
    uint32_t count = 0;

    portENTER_CRITICAL(&mux);
    if (buttonEvent)
    {
        buttonEvent = false;
        event = true;
        count = pressCount;
    }
    portEXIT_CRITICAL(&mux);

    if (event)
    {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);

        Serial.print("So lan nhan: ");
        Serial.print(count);
        Serial.print(" | LED: ");
        Serial.println(ledState ? "ON" : "OFF");
    }
}
