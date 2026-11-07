#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <cstring>

uint8_t receiverMac[] = {
    0x7C, 0xDF, 0x00, 0x43, 0xB2, 0xC1};
struct GoiTin
{
  int dem;
};

GoiTin goi = {};
uint32_t lastSend = 0;
bool transmitterReady = false;

bool isReceiverMacConfigured()
{
  for (uint8_t value : receiverMac)
  {
    if (value != 0)
    {
      return true;
    }
  }

  return false;
}

void setup()
{
  Serial.begin(115200);
  if (!WiFi.STA.begin())
  {
    Serial.println("WiFi STA initialization failed");
    return;
  }

  Serial.print("Transmitter MAC: ");
  Serial.println(WiFi.STA.macAddress());

  if (!isReceiverMacConfigured())
  {
    Serial.println("Set receiverMac to the Receiver MAC before sending");
    return;
  }

  if (esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE) != ESP_OK)
  {
    Serial.println("Failed to set Wi-Fi channel");
    return;
  }

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMac, sizeof(receiverMac));
  peer.channel = 1;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK)
  {
    Serial.println("Failed to add receiver");
    return;
  }

  transmitterReady = true;
  Serial.println("Transmitter ready");
}

void loop()
{
  if (!transmitterReady)
  {
    delay(100);
    return;
  }

  if (millis() - lastSend >= 2000)
  {
    lastSend = millis();
    goi.dem++;

    esp_err_t result = esp_now_send(receiverMac, reinterpret_cast<const uint8_t *>(&goi), sizeof(goi));

    Serial.print("Sent count: ");
    Serial.print(goi.dem);
    Serial.print(" | result: ");
    Serial.println(result == ESP_OK ? "OK" : "ERROR");
  }
}
