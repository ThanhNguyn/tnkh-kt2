// BAI 2 - Debounce bang millis() trong ISR. Thu DEBOUNCE_MS = 5 / 50 / 500
#include <Arduino.h>

#define BUTTON_PIN 4
#define LED_PIN 2
#ifndef DEBOUNCE_MS
#define DEBOUNCE_MS 50
#endif

volatile uint32_t rawEdgeCount = 0;        // suon xuong tho (chua chong doi)
volatile uint32_t debouncedCount = 0;      // so lan nhan thuc su
volatile uint32_t lastInterruptTime = 0;

void IRAM_ATTR handleInterrupt() {
  rawEdgeCount++;
  uint32_t now = millis();
  if (now - lastInterruptTime >= DEBOUNCE_MS) {
    debouncedCount++;
    lastInterruptTime = now;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);
  Serial.printf("DEBOUNCE_MS = %d\n", DEBOUNCE_MS);
}

void loop() {
  static uint32_t lastShown = 0;
  noInterrupts();
  uint32_t raw = rawEdgeCount, deb = debouncedCount;
  interrupts();
  if (deb != lastShown) {
    lastShown = deb;
    digitalWrite(LED_PIN, deb & 1);
    Serial.printf("raw=%lu  debounced=%lu\n", (unsigned long)raw, (unsigned long)deb);
  }
}
