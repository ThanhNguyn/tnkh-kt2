#include <Arduino.h>

#define BUTTON_PIN 4   
#define LED_PIN    2

volatile uint32_t isrCount = 0;   

void IRAM_ATTR handleInterrupt() {
  isrCount++;   
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("=== BAI 2 - VD1: Quan sat hien tuong nay phim (KHONG chong doi) ===");
  Serial.println("Nhan nut 1 lan va xem isrCount tang bao nhieu don vi.");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);
}

void loop() {

  int pinState = digitalRead(BUTTON_PIN);

  digitalWrite(LED_PIN, pinState == LOW ? HIGH : LOW);

  Serial.print(">pinState:");
  Serial.println(pinState);
  Serial.print(">isrCount:");
  Serial.println(isrCount);

  delay(2); 
}
