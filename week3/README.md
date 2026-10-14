# Week 3 — Tầng mạng (LT3)

Nguồn đề: `LT3-tang-mang.pdf` trang 71 + demo web/DHT/MQTT trong slide.

## Đề chuẩn (BTVN)

- Nạp web server bật/tắt LED, vào bằng điện thoại, chụp màn hình web + Serial in IP.
- Mở rộng (tùy chọn): thêm nhiệt/ẩm DHT11 lên web; hoặc thử AP khi không có router.

## Kiến thức trọng tâm

- LAN/WAN/PAN; ESP32 làm chủ LAN+PAN (Wi-Fi, BLE, ESP-NOW).
- Wi-Fi AP (tự phát) vs STA (nối router) — không router thì dùng AP.
- HTTP request–response; ESP32 dựng server bằng `WiFi.h + WebServer.h`: `server(80)`, `server.on(route,ham)`, `handleClient()` trong `loop()`.
- Route `/`, `/on`, `/off`, `/toggle`, `/sensor`; mã 200/404.
- ESP-NOW: 2 bo nói chuyện không router. BLE: chạy pin tầm ngắn.

## Map project trong folder này

- [bai1-web-server](./bai1-web-server) — STA `Wokwi-GUEST` + server 80 + `/on/off/toggle` LED GPIO2 (303 redirect về `/`).
- [bai2-dht11-web-server](./bai2-dht11-web-server) — thêm DHT GPIO15 + `/sensor` JSON + `fetch()` 2.5s. Chú ý: GPIO15 là strapping → nên chuyển DHT về GPIO27; `diagram.json` đang thiếu LED GPIO2.
- [bai3-ap-mode](./bai3-ap-mode) — AP `ThanhESP32` / `Thanh@2026` / `192.168.4.1` + web ON/OFF/TOGGLE (text/plain cho `fetch()`), nhánh `WOKWI_SIM` chạy AP_STA + STA timeout 10s.

## Minh chứng

- [ ] Ảnh web trên điện thoại + Serial in IP
- [ ] (Mở rộng) DHT trên web / AP không router

## Ôn tập (trang 70)

1. Phân biệt AP vs STA, khi nào phải AP?
2. STA thì lấy IP web server ở đâu?
3. 4 thành phần HTTP request + ý nghĩa 200/404?
4. `server.on("/toggle",...)` và `handleClient()` làm gì? Vì sao trong `loop()`?
5. Khi nào ESP-NOW thay Wi-Fi? BLE thay Wi-Fi?
6. HTTP khác MQTT thế nào? Cái nào hợp dashboard nhiều thiết bị?
