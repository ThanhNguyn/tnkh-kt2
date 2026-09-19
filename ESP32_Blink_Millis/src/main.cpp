#include <Arduino.h>

#define LED_PIN 2

const unsigned long INTERVAL = 500;

bool ledState = LOW;
unsigned long previousMillis = 0;

void setup() {
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(115200);

    digitalWrite(LED_PIN, LOW);
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= INTERVAL) {
        previousMillis = currentMillis;

        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);

        Serial.print("millis = ");
        Serial.print(currentMillis);
        Serial.print(" ms | LED = ");
        Serial.println(ledState ? "ON" : "OFF");
    }
}