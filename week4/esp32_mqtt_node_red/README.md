# BT Tuần 4 — ESP32 + DHT22 + REST JSON + MQTT + Node-RED

## 1. Build/Run PlatformIO

```bash
pio run
```

Build xong để Wokwi chạy với `wokwi.toml`.

## 2. Wokwi Wi-Fi

SSID: `Wokwi-GUEST`
Password: để trống
Channel: 6

## 3. MQTT

Broker: `broker.hivemq.com:1883`

Sensor topic:
`tnkh/week4/esp32/sensor`

LED command topic:
`tnkh/week4/esp32/led`

Payload sensor ví dụ:
`{"temperature":24,"humidity":55}`

LED command: `ON` / `OFF`

## 4. HTTP API

Sau khi ESP32 chạy, mở IP mà Serial Monitor in ra.

Ví dụ:
`http://<ESP32-IP>/api/sensor`

Kết quả JSON:
`{"temperature":24,"humidity":55,"unit_temperature":"C","unit_humidity":"%","mqtt_topic":"tnkh/week4/esp32/sensor"}`

## 5. Node-RED

Cài node dashboard nếu chưa có:

```bash
npm install node-red-dashboard
```

Mở Node-RED và import `node-red-flow.json`.
Dashboard mặc định:
`http://localhost:1880/ui`

## 6. Minh chứng cần chụp

1. Browser mở `/api/sensor` thấy JSON.
2. `http://localhost:1880/ui` thấy chart nhiệt độ/độ ẩm realtime.
3. Phần mở rộng: bấm ON/OFF trên dashboard và LED GPIO2 đổi trạng thái.
