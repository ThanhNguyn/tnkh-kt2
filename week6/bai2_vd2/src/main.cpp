#include <Arduino.h>

#define LED_PIN 2

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("Thuc day do: TIMER");
  }
  else
  {
    Serial.println("Thuc day do: CAP NGUON / RESET");
  }

  Serial.println("ESP32 chuan bi vao Deep Sleep 5 giay...");
  esp_sleep_enable_timer_wakeup(5ULL * 1000000ULL);
  esp_deep_sleep_start();
}

void loop()
{
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
