#include <Arduino.h>

#define LED_PIN 2
#define POT_PIN 34

#define LEDC_CHANNEL 0
#define LEDC_FREQUENCY 5000
#define LEDC_RESOLUTION 8

#define TIMER_DIVIDER 80
#define TIMER_INTERVAL_US 100000UL  // 100 ms

hw_timer_t *timer = nullptr;
volatile bool timerFlag = false;

void IRAM_ATTR timerISR()
{
    timerFlag = true;
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);

    // LEDC PWM: 5 kHz, 8-bit
    ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, LEDC_RESOLUTION);
    ledcAttachPin(LED_PIN, LEDC_CHANNEL);
    ledcWrite(LEDC_CHANNEL, 0);

    // Hardware Timer: 80 MHz / 80 = 1 MHz => 1 tick = 1 us
    timer = timerBegin(0, TIMER_DIVIDER, true);
    timerAttachInterrupt(timer, &timerISR, true);
    timerAlarmWrite(timer, TIMER_INTERVAL_US, true);
    timerAlarmEnable(timer);

    Serial.println("=== DEMO 2 ===");
    Serial.println("ESP32 ready");
}

void loop()
{
    if (timerFlag)
    {
        timerFlag = false;

        int raw = analogRead(POT_PIN);

        // ADC 12-bit: 0..4095 -> LEDC 8-bit: 0..255
        uint8_t duty = static_cast<uint8_t>(raw / 16);

        ledcWrite(LEDC_CHANNEL, duty);

        Serial.print("ADC raw: ");
        Serial.print(raw);
        Serial.print(" | PWM: ");
        Serial.println(duty);
    }
}
