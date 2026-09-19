#include <Arduino.h>

#define LED_PIN 2
#define BUTTON_PIN 4
#define PWM_CHANNEL 0
#define PWM_FREQUENCY 5000
#define PWM_RESOLUTION 8

const uint8_t brightnessLevels[] = {0, 64, 128, 192, 255};
const size_t brightnessCount = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);

uint8_t brightnessIndex = 0;
bool lastButtonState = HIGH;

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(LED_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, brightnessLevels[brightnessIndex]);

    Serial.println("=== LED SANG DAN + NUT ===");
    Serial.println("ESP32 ready");
    Serial.println("Nhan nut de tang do sang LED");
}

void loop()
{
    const bool buttonState = digitalRead(BUTTON_PIN);

    if (lastButtonState == HIGH && buttonState == LOW)
    {
        brightnessIndex = (brightnessIndex + 1) % brightnessCount;
        const uint8_t duty = brightnessLevels[brightnessIndex];

        ledcWrite(PWM_CHANNEL, duty);

        Serial.print("Muc sang: ");
        Serial.print(duty);
        Serial.print(" / 255  (");
        Serial.print((duty * 100) / 255);
        Serial.println("%)");

        delay(50);
    }

    lastButtonState = buttonState;
}
