#include <Arduino.h>

constexpr uint8_t STRAPPING_PIN = 12;
constexpr unsigned long BLINK_MS = 500;

void setup() {
  Serial.begin(115200);
  delay(1000);

  const int strapState = digitalRead(STRAPPING_PIN);

  Serial.println();
  Serial.println("=== DEMO 4 - ESP32 GPIO12 STRAPPING PIN ===");
  Serial.println("ESP32 DevKit v1 | GPIO12 / MTDI");
  Serial.printf("GPIO12 level observed after boot: %s\n", strapState ? "HIGH" : "LOW");
  Serial.println();
  Serial.println("Hardware note:");
  Serial.println("GPIO12 is sampled during reset/boot and affects VDD_SPI configuration.");
  Serial.println("On classic ESP32 boards, pulling GPIO12 HIGH during boot can select 1.8 V flash supply.");
  Serial.println("A 3.3 V flash board may then fail to read flash and show a boot error such as 'flash read err'.");
  Serial.println();
  Serial.println("Wokwi note: this simulation does NOT reproduce the physical flash-voltage failure.");
  Serial.println("It demonstrates the GPIO12 pull-up and the GPIO behavior after boot.");
  Serial.println();

  pinMode(STRAPPING_PIN, OUTPUT);
  digitalWrite(STRAPPING_PIN, LOW);
}

void loop() {
  digitalWrite(STRAPPING_PIN, HIGH);
  delay(BLINK_MS);
  digitalWrite(STRAPPING_PIN, LOW);
  delay(BLINK_MS);
}
