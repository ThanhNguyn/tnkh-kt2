#include <Arduino.h>

#define POT_PIN 34
#define SAMPLE_COUNT 16
#define SAMPLE_INTERVAL_MS 500

void setup()
{
    Serial.begin(115200);

    analogReadResolution(12);
    analogSetPinAttenuation(POT_PIN, ADC_11db);

    Serial.println("=== DEMO 3 - ADC RAW vs CALIBRATED ===");
    Serial.println("GPIO34 | 16-sample average");
    Serial.println("ESP32 ready");
}

void loop()
{
    uint32_t rawSum = 0;
    uint32_t calibratedMvSum = 0;

    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        rawSum += analogRead(POT_PIN);
        calibratedMvSum += analogReadMilliVolts(POT_PIN);
    }

    const uint32_t rawAverage = rawSum / SAMPLE_COUNT;
    const uint32_t calibratedMvAverage = calibratedMvSum / SAMPLE_COUNT;

    // Theoretical calculation based on 12-bit raw ADC and 3.3 V reference.
    const uint32_t theoreticalMv =
        (rawAverage * 3300UL) / 4095UL;

    Serial.print("Raw avg: ");
    Serial.print(rawAverage);

    Serial.print(" | Theoretical: ");
    Serial.print(theoreticalMv);
    Serial.print(" mV");

    Serial.print(" | Calibrated: ");
    Serial.print(calibratedMvAverage);
    Serial.println(" mV");

    delay(SAMPLE_INTERVAL_MS);
}
