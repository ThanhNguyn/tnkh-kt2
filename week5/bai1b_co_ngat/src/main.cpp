// BAI 1b - LED fading, CO ngat ngoai: nut phan hoi NGAY bang mot cai chop LED
#include <Arduino.h>
#include "pwm_compat.h"

#define BUTTON_PIN 4
#define LED_PIN 2

volatile bool buttonPressed = false;
volatile uint32_t pressCount = 0;

void IRAM_ATTR handleInterrupt() {
  pressCount++;          // ISR ngan gon: chi dem + dat co
  buttonPressed = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pwmBegin(LED_PIN, 0, 5000, 8);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);
  Serial.println("San sang. Nhan nut de xem LED nhay nhanh bao hieu ngat.");
}

void loop() {
  static int brightness = 0, step = 1;
  if (buttonPressed) {
    buttonPressed = false;
    Serial.printf("Ngat! so lan nhan = %lu\n", (unsigned long)pressCount);
    pwmWrite(LED_PIN, 0, 0);   delay(80);   // nhay tat nhanh
    pwmWrite(LED_PIN, 0, 255); delay(80);   // nhay sang nhanh
  }
  brightness += step;
  if (brightness <= 0 || brightness >= 255) step = -step;
  pwmWrite(LED_PIN, 0, brightness);
  delay(10);
}
