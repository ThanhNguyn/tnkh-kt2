#include <Arduino.h>

constexpr uint8_t ADC_PIN = 34;
constexpr uint8_t SAMPLE_COUNT = 16;
constexpr float VREF_THEORY_MV = 3300.0f;
constexpr float ADC_MAX = 4095.0f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);

  Serial.println();
  Serial.println("=== DEMO 3 - ADC RAW vs THEORETICAL vs CALIBRATED ===");
  Serial.println("ESP32 DevKit v1 | GPIO34 (ADC1)");
  Serial.println("Samples per reading: 16");
  Serial.println();
}

void loop() {
  uint32_t rawSum = 0;
  uint32_t calibratedSumMv = 0;

  for (uint8_t i = 0; i < SAMPLE_COUNT; ++i) {
    rawSum += analogRead(ADC_PIN);
    calibratedSumMv += analogReadMilliVolts(ADC_PIN);
    delayMicroseconds(500);
  }

  const float rawAverage = static_cast<float>(rawSum) / SAMPLE_COUNT;
  const float theoreticalMv = rawAverage * VREF_THEORY_MV / ADC_MAX;
  const float calibratedMv = static_cast<float>(calibratedSumMv) / SAMPLE_COUNT;

  Serial.printf("raw(avg 16): %4.0f | theory: %7.1f mV | calibrated: %7.1f mV\n",
                rawAverage, theoreticalMv, calibratedMv);

  delay(500);
}
