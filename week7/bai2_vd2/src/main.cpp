
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_sleep.h"
#include "esp_system.h"

#define LED_PIN 2

#define WOKWI_SIMULATION 1

const char *AP_SSID = "ESP32-DeepSleep";
const char *AP_PASSWORD = "esp32sleep";

const uint64_t SLEEP_TIME_US = 5ULL * 1000000ULL;

WebServer server(80);

uint32_t lastBlink = 0;
bool ledState = false;

void kiemTraNguyenNhanThucDay()
{
  esp_sleep_wakeup_cause_t reason =
      esp_sleep_get_wakeup_cause();

  if (reason == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("Thuc day do: TIMER");
  }
  else
  {
    Serial.println("Thuc day do: CAP NGUON / RESET");
  }

  Serial.print("Reset reason: ");
  Serial.println((int)esp_reset_reason());
}

void khoiTaoWiFi()
{
#if WOKWI_SIMULATION

  // Wokwi ket noi vao mang ao Wokwi-GUEST
  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "", 6);

  Serial.print("Dang ket noi Wokwi-GUEST");

  uint32_t startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 15000)
  {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wi-Fi da ket noi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Khong the ket noi Wi-Fi Gateway!");
  }

#else

  WiFi.mode(WIFI_AP);

  if (WiFi.softAP(AP_SSID, AP_PASSWORD))
  {
    Serial.println("Wi-Fi AP da khoi tao!");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);

    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    Serial.println("Loi khoi tao Wi-Fi AP!");
  }

#endif
}

void khoiTaoWebServer()
{
  server.on("/", HTTP_GET, []()
            { server.send(
                  200,
                  "text/plain",
                  "ESP32 da thuc day va dang hoat dong!"); });

  server.begin();

  Serial.println("Web Server da khoi dong!");
}

void setup()
{
  Serial.begin(115200);
  delay(300);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println();
  Serial.println("--- BAI 2 - VI DU 2: DEEP SLEEP ---");

  kiemTraNguyenNhanThucDay();

#if WOKWI_SIMULATION

  Serial.println("Wokwi mode: bo qua Deep Sleep.");

#else

  if (esp_reset_reason() == ESP_RST_POWERON)
  {
    Serial.println("ESP32 vao Deep Sleep 5 giay...");

    digitalWrite(LED_PIN, LOW);
    Serial.flush();

    esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
    esp_deep_sleep_start();
  }

#endif

  khoiTaoWiFi();
  khoiTaoWebServer();

  Serial.println("ESP32 da san sang!");
}

void loop()
{
  server.handleClient();

  if (millis() - lastBlink >= 500)
  {
    lastBlink = millis();

    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }

  delay(1);
}
