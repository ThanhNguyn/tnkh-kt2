// BAI 3a - ADC1 (GPIO 34) -> loc trung binh truot -> LEDC (GPIO 18)
// Doi USE_FILTER 0/1 de chup so sanh THO vs DA LOC
#include <Arduino.h>
#include "pwm_compat.h"

#define POT_PIN 34      // ADC1_CH6, input-only
#define LED_PIN 18
#define N 8
#ifndef USE_FILTER
#define USE_FILTER 1
#endif
#ifndef SIM_NOISE
#define SIM_NOISE 0
#endif

int buf[N] = {0};
int idx = 0;
long sum = 0;

int loc(int m) {                 // trung binh truot N mau
  sum -= buf[idx];
  buf[idx] = m;
  sum += m;
  idx = (idx + 1) % N;
  return sum / N;
}

void setup() {
  Serial.begin(115200);
  pwmBegin(LED_PIN, 0, 5000, 8);
}

void loop() {
  int tho = analogRead(POT_PIN);                       // 0..4095
#if SIM_NOISE
  tho = constrain(tho + (int)random(-60, 61), 0, 4095);
#endif
  int l = loc(tho);
  int v = USE_FILTER ? l : tho;
  pwmWrite(LED_PIN, 0, map(v, 0, 4095, 0, 255));
  Serial.print(">tho:"); Serial.println(tho);          // Teleplot
  Serial.print(">loc:"); Serial.println(l);
  delay(20);
}
