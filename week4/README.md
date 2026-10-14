# Week 4 — Đề bài code

## ESP32 → REST + MQTT → Node-RED
- DHT22 GPIO4 đọc mỗi 2s, LED GPIO2.
- `GET /api/sensor` trả JSON nhiệt/ẩm + unit + topic, 503 khi chưa có số liệu thật.
- Publish `tnkh/week4/esp32/sensor` `{"temperature":..,"humidity":..}`, subscribe `tnkh/week4/esp32/led` lệnh ON/OFF.
- Broker `broker.hivemq.com:1883`, Wi-Fi `Wokwi-GUEST` ch6.
- Node-RED: chart + gauge nhiệt/ẩm + nút LED → `http://localhost:1880/ui`.
- Project: [esp32_mqtt_node_red](./esp32_mqtt_node_red) (flow: `node-red-flow.json`)
