# TH4 – Bài 2: ESP32 MQTT Publisher

Project ESP32 độc lập để đọc DHT và publish telemetry JSON lên MQTT broker theo chu kỳ 2 giây.

## Tính năng

- Kết nối Wi-Fi và MQTT broker.
- Publish `id`, `temp`, `hum`, `uptime`, `rssi` dưới dạng JSON.
- Topic: `aet2022/<SV_ID>/telemetry`.
- Client ID lấy từ MAC address để giảm khả năng trùng client giữa các ESP32.
- Có vòng lặp duy trì kết nối Wi-Fi/MQTT và gọi `mqtt.loop()` thường xuyên.

## Cấu trúc

```text
TH4-Bai2-MQTT-Publisher/
├── .vscode/extensions.json
├── diagram.json
├── include/project_config.h
├── platformio.ini
├── src/main.cpp
└── wokwi.toml
```

## Kết nối cảm biến

| DHT22 | ESP32 / breadboard |
|---|---|
| VCC | 3V3 rail |
| GND | GND rail |
| DATA | GPIO4 |


## Cấu hình

Sửa `include/project_config.h`: `SV_ID`, `MQTT_HOST`, `MQTT_PORT`, `WIFI_SSID`, `WIFI_PASSWORD`. Mặc định dùng `broker.hivemq.com:1883` và `Wokwi-GUEST`.

## Build và chạy

```powershell
pio run
```

Chạy `Wokwi: Start Simulator`, bật Wokwi Private IoT Gateway nếu cần truy cập Internet/MQTT. Mở Serial Monitor `115200` để xem trạng thái kết nối và log publish.

## Kiểm tra bằng MQTT Explorer 

- Web: [Demo](https://www.hivemq.com/demos/websocket-client)
- Host: `broker.hivemq.com`
- Port: `1883`
- Subscribe: `aet2022/<SV_ID>/telemetry`
- Kỳ vọng nhận JSON khoảng 2 giây/lần.

Payload mẫu:

```json
{
  "id": "YOUR_STUDENT_ID",
  "temp": 24.0,
  "hum": 55.0,
  "uptime": 12,
  "rssi": -45
}
```

## Thư viện

PlatformIO tự cài ArduinoJson, PubSubClient, Adafruit DHT sensor library và Adafruit Unified Sensor theo `platformio.ini`.

## Lưu ý kiểm thử

Cấu trúc và file JSON đã được kiểm tra tĩnh. Chưa xác nhận build PlatformIO hoặc kết nối broker trực tiếp trong môi trường này.
