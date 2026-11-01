#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

constexpr uint8_t LED_PIN = 2;

#ifdef WOKWI
const char *SSID = "Wokwi-GUEST";
const char *PASS = "";
constexpr int WIFI_CHANNEL = 6;
#else
const char *SSID = "TEN_WIFI";
const char *PASS = "MAT_KHAU";
constexpr int WIFI_CHANNEL = 0;
#endif

WebServer server(80);
bool ledOn = false;

void setLed(bool state)
{
  ledOn = state;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  Serial.println(ledOn ? "LED: BAT" : "LED: TAT");
}

String page()
{
  String html =
      "<!DOCTYPE html><html><head><meta charset='utf-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>ESP32 LED</title>"
      "<style>"
      "body{font-family:sans-serif;text-align:center;margin-top:48px}"
      ".st{display:inline-block;padding:12px 28px;border-radius:8px;"
      "font-size:26px;font-weight:bold;color:#fff}"
      ".on{background:#2e7d32}.off{background:#c62828}"
      "a button{font-size:22px;padding:12px 30px;margin:8px;border:0;"
      "border-radius:8px;background:#1976d2;color:#fff}"
      "</style></head><body><h1>ESP32 LED</h1>";
  html += ledOn ? "<div class='st on'>LED đang BẬT</div>"
                : "<div class='st off'>LED đang TẮT</div>";
  html += "<p><a href='/on'><button>Bật</button></a>"
          "<a href='/off'><button>Tắt</button></a>"
          "<a href='/toggle'><button>Toggle</button></a></p>"
          "</body></html>";
  return html;
}

void redirectHome()
{
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleRoot() { server.send(200, "text/html; charset=utf-8", page()); }
void handleOn()
{
  setLed(true);
  redirectHome();
}
void handleOff()
{
  setLed(false);
  redirectHome();
}
void handleToggle()
{
  setLed(!ledOn);
  redirectHome();
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  setLed(false);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS, WIFI_CHANNEL);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print('.');
  }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/toggle", handleToggle);
  server.begin();
}

void loop()
{
  server.handleClient();
}
