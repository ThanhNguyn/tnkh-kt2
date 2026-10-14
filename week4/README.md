# Week 4 — Tầng API / ứng dụng (LT4)

Nguồn đề: `LT4-tang-api-ung-dung.pdf` trang 43.

## Đề chuẩn (BTVN)

- Cho ESP32 (a) trả `/api/sensor` JSON nhiệt/ẩm VÀ (b) publish MQTT lên dashboard (chọn 1: Node-RED / Adafruit IO / Blynk).
- Chụp JSON trên trình duyệt + dashboard realtime.
- Mở rộng: nút dashboard ĐK LED qua MQTT; hoặc WiFiManager cấu hình Wi-Fi qua web.

## Kiến thức trọng tâm

- API JSON cho máy đọc, khác HTML cho người đọc. REST = tài nguyên + phương thức (GET/POST/PUT) + endpoint.
- MQTT publish/subscribe qua broker — hợp nhiều thiết bị gửi liên tục; REST hợp hỏi–đáp.
- Dashboard: Node-RED (kiểm soát) / Blynk-Adafruit IO (nhanh) / ThingsBoard (quy mô).
- Sản phẩm: WiFiManager cấu hình Wi-Fi qua web, Preferences lưu bền, OTA nạp từ xa.

## Map project trong folder này

- [esp32_mqtt_node_red](./esp32_mqtt_node_red) — DHT22 GPIO4 2s + LED GPIO2:
  - `GET /api/sensor` JSON + 503 khi chưa có số liệu thật (không giả số 0).
  - Publish `tnkh/week4/esp32/sensor` `{"temperature":..,"humidity":..}` + topic phụ `.../api/sensor`.
  - Subscribe `tnkh/week4/esp32/led` lệnh `ON/OFF`.
  - Broker `broker.hivemq.com:1883`, Wi-Fi `Wokwi-GUEST` ch6, reconnect Wi-Fi/MQTT.
  - `node-red-flow.json`: chart + gauge nhiệt/ẩm + nút LED ON/OFF → `http://localhost:1880/ui`.

## Minh chứng

- [ ] Ảnh JSON `/api/sensor` trên browser
- [ ] Ảnh dashboard realtime
- [ ] (Mở rộng) LED ON/OFF qua MQTT

## Ôn tập (trang 42)

1. Vì sao app nên nhận JSON thay vì HTML?
2. 3 phương thức HTTP của REST + công dụng trên ESP32?
3. Publisher/subscriber/broker/topic là gì?
4. Khi nào MQTT thay vì REST liên tục?
5. Đồ án nhỏ cần nhanh: chọn Node-RED / Blynk-Adafruit / ThingsBoard nào?
6. WiFiManager / Preferences / OTA giải quyết gì?
