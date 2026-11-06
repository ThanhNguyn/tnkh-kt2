# TH4 – Bài 3: Node-RED MQTT Dashboard

Project độc lập cho dashboard giám sát và điều khiển ESP32 qua MQTT. Repo bao gồm cả Node-RED flow lẫn firmware ESP32 hai chiều trong `device/`, vì vậy không phải clone hoặc phụ thuộc source từ Bài 2.

## Chức năng

- Nhận telemetry nhiệt độ/độ ẩm/uptime từ ESP32 qua MQTT.
- Hiển thị gauge nhiệt độ, gauge độ ẩm, chart lịch sử và uptime.
- Nút LED ON/OFF gửi payload `1`/`0` về ESP32.
- ESP32 subscribe `aet2022/<SV_ID>/cmd/led`, điều khiển LED GPIO14 và publish trạng thái LED trong telemetry.

## Cấu trúc repo

```text
TH4-Bai3-NodeRED-Dashboard/
├── .gitignore
├── configure.ps1
├── device/
│   ├── diagram.json
│   ├── include/project_config.h
│   ├── platformio.ini
│   ├── src/main.cpp
│   └── wokwi.toml
├── docs/
├── flows/th4-dashboard-flow.json
└── package.json
```

## MQTT contract

| Direction | Topic | Payload |
|---|---|---|
| ESP32 → Node-RED | `aet2022/<SV_ID>/telemetry` | JSON: `id`, `temp`, `hum`, `uptime`, `rssi`, `led` |
| Node-RED → ESP32 | `aet2022/<SV_ID>/cmd/led` | `1` hoặc `0` |

Lệnh LED không retain để tránh thiết bị tự bật/tắt khi reconnect.