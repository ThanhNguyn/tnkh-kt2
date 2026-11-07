// BAI 1a - LED fading, KHONG ngat (polling): nut phan hoi tre vi loop() dang ban
#include <Arduino.h>
#include "pwm_compat.h"

#define BUTTON_PIN 4
#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pwmBegin(LED_PIN, 0, 5000, 8);
}

void loop() {
  for (int b = 0; b <= 255; b++) {
    pwmWrite(LED_PIN, 0, b);
    if (digitalRead(BUTTON_PIN) == LOW) Serial.println("button pressed");
    delay(10);
  }
  for (int b = 255; b >= 0; b--) {
    pwmWrite(LED_PIN, 0, b);
    if (digitalRead(BUTTON_PIN) == LOW) Serial.println("button pressed");
    delay(10);
  }
}
