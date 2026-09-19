#include <Arduino.h>
#include <stddef.h>

// =========================
// Pin configuration
// =========================
constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t LED1_PIN = 2;
constexpr uint8_t LED2_PIN = 5;

// =========================
// Button / PWM configuration
// =========================
constexpr uint32_t DEBOUNCE_MS = 50;
constexpr uint8_t LEDC_CHANNEL = 0;
constexpr uint32_t PWM_FREQUENCY = 5000;
constexpr uint8_t PWM_RESOLUTION = 8;

// Duty sequence: 0%, 25%, 50%, 75%, 100%
constexpr uint8_t PWM_LEVELS[] = {0, 64, 128, 192, 255};
constexpr size_t PWM_LEVEL_COUNT = sizeof(PWM_LEVELS) / sizeof(PWM_LEVELS[0]);

// =========================
// Hardware timer configuration
// 80 MHz / 80 = 1 MHz -> 1 tick = 1 us
// 250,000 us = 250 ms
// =========================
hw_timer_t *timer = nullptr;

volatile uint32_t pressCount = 0;
volatile uint32_t lastButtonInterruptMs = 0;
volatile bool timerFlag = false;

bool led2State = false;
uint32_t processedPressCount = 0;

void IRAM_ATTR buttonISR()
{
    const uint32_t now = millis();

    if (now - lastButtonInterruptMs >= DEBOUNCE_MS) {
        ++pressCount;
        lastButtonInterruptMs = now;
    }
}

void IRAM_ATTR timerISR()
{
    timerFlag = true;
}

void setup()
{
    Serial.begin(115200);

    // Button: released = HIGH, pressed = LOW.
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED2_PIN, OUTPUT);
    digitalWrite(LED2_PIN, LOW);

    // LED1: LEDC PWM, 5 kHz, 8-bit.
    ledcSetup(LEDC_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(LED1_PIN, LEDC_CHANNEL);
    ledcWrite(LEDC_CHANNEL, PWM_LEVELS[0]);

    // External interrupt on the falling edge.
    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        buttonISR,
        FALLING
    );

    // Hardware timer: 80 MHz / 80 = 1 MHz.
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &timerISR, true);
    timerAlarmWrite(timer, 250000, true);
    timerAlarmEnable(timer);

    Serial.println("=== GHEP DEMO 1 + DEMO 2 ===");
    Serial.println("ESP32 ready");
}

void loop()
{
    // Process button presses recorded by the external interrupt.
    uint32_t currentPressCount;
    noInterrupts();
    currentPressCount = pressCount;
    interrupts();

    if (currentPressCount != processedPressCount) {
        // Handle every valid press that has not been printed yet.
        while (processedPressCount < currentPressCount) {
            ++processedPressCount;
            const uint8_t duty = PWM_LEVELS[processedPressCount % PWM_LEVEL_COUNT];

            ledcWrite(LEDC_CHANNEL, duty);

            Serial.print("So lan nhan: ");
            Serial.print(processedPressCount);
            Serial.print(" | PWM: ");
            Serial.println(duty);
        }
    }

    // Process the hardware-timer event outside the ISR.
    bool timerEvent = false;
    noInterrupts();
    if (timerFlag) {
        timerFlag = false;
        timerEvent = true;
    }
    interrupts();

    if (timerEvent) {
        led2State = !led2State;
        digitalWrite(LED2_PIN, led2State ? HIGH : LOW);
    }
}