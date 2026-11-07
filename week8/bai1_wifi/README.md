# TH3 — Bài 1: Wi-Fi Access Point và Station

Project triển khai hai ví dụ Wi-Fi độc lập theo nội dung Bài 1:

- **Access Point (AP):** tạo mạng Wi-Fi `MyESP32AP` và cung cấp địa chỉ AP.
- **Station (STA):** kết nối vào mạng Wi-Fi được khai báo và hiển thị thông tin kết nối.
- **AP + STA (APSTA):** khởi tạo SoftAP và kết nối Station đồng thời trên cùng ESP32-S3.

Hai ví dụ AP và STA độc lập được biên dịch thành hai firmware riêng; APSTA là môi trường tích hợp bổ sung.

## PlatformIO targets

| Environment | Target | Firmware |
|---|---|---|
| `classic_ap` | ESP32 classic (`esp32dev`) | AP |
| `classic_sta` | ESP32 classic (`esp32dev`) | STA |
| `emu_ap` | ESP32-S3 (`esp32-s3-devkitc-1`) | AP |
| `emu_sta` | ESP32-S3 (`esp32-s3-devkitc-1`) | STA |
| `emu_apsta` | ESP32-S3 (`esp32-s3-devkitc-1`) | AP + STA simultaneously |

## Source layout

```text
include/wifi_config.h
src/ap/main.cpp
src/sta/main.cpp
src/apsta/main.cpp
scripts/merge_emulator_image.py
platformio.ini
```
