#include <Arduino.h>
#include <WiFi.h>

#if defined(WIFI_AP_MODE)

const char *ssid = "MyESP32AP";
const char *password = "password123";

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);

  if (WiFi.softAP(ssid, password))
  {
    Serial.println("Access Point started");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    Serial.println("Access Point start failed");
  }
}

void loop()
{
  static uint32_t lastReport = 0;

  if (millis() - lastReport >= 2000)
  {
    lastReport = millis();
    Serial.print("Connected stations: ");
    Serial.println(WiFi.softAPgetStationNum());
  }
}

#else

#if defined(WOKWI_SIMULATION)
const char *ssid = "Wokwi-GUEST";
const char *password = "";
#else
const char *ssid = "MyESP32AP";
const char *password = "password123";
#endif

void connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
}

void setup()
{
  Serial.begin(115200);
  connectWiFi();
}

void loop()
{
  static uint32_t lastReport = 0;
  static bool wasConnected = false;

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!wasConnected)
    {
      wasConnected = true;
      Serial.println("Wi-Fi connected");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
  }
  else
  {
    wasConnected = false;

    if (millis() - lastReport >= 10000)
    {
      lastReport = millis();
      Serial.println("Wi-Fi disconnected, reconnecting");
      WiFi.reconnect();
    }
  }

  delay(10);
}

#endif
