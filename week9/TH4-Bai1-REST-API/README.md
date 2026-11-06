# TH4 – Bài 1: ESP32 REST API

ESP32 đọc cảm biến DHT và cung cấp dữ liệu qua HTTP REST endpoint `GET /api/sensor`. Project độc lập, mở trực tiếp thư mục này trong VS Code/PlatformIO.

## Tính năng

- ESP32 DevKit V1, framework Arduino.
- Đọc nhiệt độ/độ ẩm bằng DHT22 trong Wokwi (có thể đổi sang DHT11 khi dùng phần cứng thật).
- HTTP server chạy cổng `80`, endpoint `GET /api/sensor`.
- Trả JSON gồm `id`, `rssi`, `uptime`, `sensor_ok`, `temp`, `hum`; lỗi cảm biến trả HTTP `503` với JSON báo lỗi.
- Wokwi forward `localhost:8180` → ESP32 `target:80`.

## Cấu trúc

```text
TH4-Bai1-REST-API/
├── .vscode/extensions.json
├── diagram.json
├── include/project_config.h
├── platformio.ini
├── src/main.cpp
└── wokwi.toml
```

## Phần cứng và kết nối

| DHT22 | ESP32 / breadboard |
|---|---|
| VCC | 3V3 rail |
| GND | GND rail |
| DATA | GPIO4 |

Sơ đồ đặt ESP32 bên trái, breadboard bên phải và cảm biến ở khu vực riêng, không chồng linh kiện. Bài 1 không đưa LED thừa vào mạch.

## Cấu hình

Mở `include/project_config.h`: thay `YOUR_STUDENT_ID` bằng mã sinh viên/nhóm. Wokwi dùng sẵn `Wokwi-GUEST`. Khi chạy ESP32 thật, sửa `WIFI_SSID` và `WIFI_PASSWORD`. Không commit mật khẩu Wi-Fi thật lên repo công khai.

## Build và chạy

```powershell
pio run
```

Trong VS Code: `Wokwi: Start Simulator`. Mở Serial Monitor ở `115200` baud. Sau khi ESP32 kết nối Wi-Fi và server khởi động, kiểm tra từ PowerShell: 

```powershell
curl.exe http://localhost:8180/api/sensor
```

Hoặc mở `http://localhost:8180/api/sensor` trên trình duyệt. JSON mẫu (giá trị đo thay đổi theo cảm biến):

```json
{
  "id": "YOUR_STUDENT_ID",
  "rssi": -45,
  "uptime": 12,
  "sensor_ok": true,
  "temp": 24.0,
  "hum": 55.0
}
```

## Thư viện

PlatformIO tự cài thư viện đã khai báo trong `platformio.ini`: ArduinoJson, Adafruit DHT sensor library và Adafruit Unified Sensor.

## Lưu ý kiểm thử

Source/configuration đã được đóng gói và JSON đã được kiểm tra cú pháp. Chưa xác nhận build PlatformIO hoặc chạy mô phỏng Wokwi trong môi trường này; cần chạy `pio run` và test endpoint trên máy thực tế.
