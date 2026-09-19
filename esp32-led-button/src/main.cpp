#include <Arduino.h>

#define LED_PIN 5
#define BUTTON_PIN 4

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    digitalWrite(LED_PIN, LOW);
}

void loop() {
    int buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("PRESSED");
    } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("RELEASED");
    }

    delay(50);
}