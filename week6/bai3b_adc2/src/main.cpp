// BAI 3b - Giong 3a nhung dung ADC2 (GPIO 15, chan strapping) + bat Wi-Fi
// Quan sat: ADC2 doc sai / ra 0 khi Wi-Fi bat -> ghi danh gia
#include <Arduino.h>
#include <WiFi.h>
#include "pwm_compat.h"

#define POT_PIN 15      // ADC2_CH3, chan strapping
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

int loc(int m) {
  sum -= buf[idx];
  buf[idx] = m;
  sum += m;
  idx = (idx + 1) % N;
  return sum / N;
}

void setup() {
  Serial.begin(115200);
  pwmBegin(LED_PIN, 0, 5000, 8);
  WiFi.begin("Wokwi-GUEST", "", 6);   // bat Wi-Fi -> ADC2 bi anh huong
}

void loop() {
  int tho = analogRead(POT_PIN);
#if SIM_NOISE
  tho = constrain(tho + (int)random(-60, 61), 0, 4095);
#endif
  int l = loc(tho);
  int v = USE_FILTER ? l : tho;
  pwmWrite(LED_PIN, 0, map(v, 0, 4095, 0, 255));
  Serial.print(">tho:"); Serial.println(tho);
  Serial.print(">loc:"); Serial.println(l);
  delay(20);
}
