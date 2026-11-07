#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <cstring>

struct GoiTin
{
  int dem;
};

GoiTin goi;

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
  if (len != sizeof(GoiTin))
  {
    return;
  }

  memcpy(&goi, data, sizeof(goi));

  Serial.printf(
      "Nhan tu %02X:%02X:%02X:%02X:%02X:%02X | Nhan dem = %d\n",
      info->src_addr[0],
      info->src_addr[1],
      info->src_addr[2],
      info->src_addr[3],
      info->src_addr[4],
      info->src_addr[5],
      goi.dem);
}

void setup()
{
  Serial.begin(115200);
  if (!WiFi.STA.begin())
  {
    Serial.println("WiFi STA initialization failed");
    return;
  }

  if (esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE) != ESP_OK)
  {
    Serial.println("Failed to set Wi-Fi channel");
    return;
  }

  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.STA.macAddress());

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  if (esp_now_register_recv_cb(onReceive) != ESP_OK)
  {
    Serial.println("Receive callback registration failed");
    return;
  }

  Serial.println("Receiver ready");
}

void loop()
{
  delay(100);
}
