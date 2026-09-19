#include <Arduino.h>

#define ADC_PIN 34

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);

    Serial.println("ESP32 ADC started");
}

void loop() {
    uint32_t voltage = analogReadMilliVolts(ADC_PIN);

    Serial.print("Voltage = ");
    Serial.print(voltage);
    Serial.println(" mV");

    delay(200);
}