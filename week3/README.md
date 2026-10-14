# Week 3 — Đề bài code

## Bài 1: Web Server + LED (STA)
- STA `Wokwi-GUEST`, server port 80.
- Route `/` web, `/on` bật LED, `/off` tắt LED, `/toggle` đảo LED GPIO2.
- Project: [bai1-web-server](./bai1-web-server)

## Bài 2: DHT Web Server
- Thêm DHT + `/sensor` trả JSON `{"temperature":..,"humidity":..}`, web `fetch()` 2.5s.
- Route `/on/off/toggle` trả `text/plain` ON/OFF.
- Project: [bai2-dht11-web-server](./bai2-dht11-web-server)

## Bài 3: AP Mode
- AP `ThanhESP32` / `Thanh@2026` / `192.168.4.1`, web ON/OFF/TOGGLE LED GPIO2.
- Nhánh `WOKWI_SIM`: AP_STA + STA timeout 10s.
- Project: [bai3-ap-mode](./bai3-ap-mode)
