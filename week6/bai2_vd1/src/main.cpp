#include <Arduino.h>

#define LED_PIN 2

RTC_DATA_ATTR int soLanThuc = 0;
int demThuong = 0;

void lanDeepSleep()
{
  soLanThuc++;
  demThuong++;

  Serial.printf("\n[Truoc khi ngu] soLanThuc (RTC) = %d | demThuong (RAM) = %d\n", soLanThuc, demThuong);
  Serial.println("ESP32 bat dau ngu 10 giay...");
  Serial.flush();

  digitalWrite(LED_PIN, LOW);
  esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);
  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("\n============================================================================");
  Serial.println("ESP32 vua thuc day (hoac vua Reset)!");
  Serial.printf("[Gia tri vua nap] soLanThuc = %d | demThuong = %d\n", soLanThuc, demThuong);
  Serial.println("============================================================================");

  delay(3000);
  lanDeepSleep();
}

void loop()
{
  // put your main code here, to run repeatedly:
}

// put function definitions here:
