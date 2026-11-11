# AET2022 – TH4: REST API, MQTT và Node-RED

**Học phần:** Thí nghiệm Khoa học Kỹ thuật 2 (AET2022)  
**Buổi thực hành:** TH4  
**Sinh viên:** Nguyễn Tuấn Thành  
**MSSV / namespace MQTT:** `25112107`  
**Repository:** [ThanhNguyn/tnkh-kt2](https://github.com/ThanhNguyn/tnkh-kt2)  
**Thư mục:** [`week9/`](https://github.com/ThanhNguyn/tnkh-kt2/tree/main/week9)

> Phạm vi minh chứng trong repository hiện tại: ESP32 được chạy trên Wokwi; MQTT dùng HiveMQ public broker; Node-RED và MQTT Explorer chạy trên máy tính. Chưa có bằng chứng kiểm thử bằng kit ESP32 vật lý. Những phần chưa thể tái chạy trực tiếp từ `main` được ghi rõ ở mục **Repository gaps**.

## 1. Tổng quan

TH4 trong repository được tổ chức thành bốn project, theo luồng từ cung cấp dữ liệu REST đến gửi/giám sát telemetry bằng MQTT:

| Bài | Nội dung | Thành phần chính | Kết quả / mục đích |
|---|---|---|---|
| 1 | ESP32 REST API | ESP32, DHT22, HTTP, JSON | Đọc cảm biến và trả JSON qua `GET /api/sensor` |
| 2 | MQTT Publisher | ESP32, DHT22, PubSubClient, HiveMQ | Publish telemetry JSON định kỳ lên broker |
| 3 | Node-RED Dashboard | ESP32 hai chiều, MQTT, Node-RED Dashboard | Hiển thị telemetry realtime và gửi lệnh bật/tắt LED |
| 4 | MQTT Explorer | MQTT Explorer, HiveMQ, firmware Bài 3 | Subscribe/quan sát JSON, lịch sử, biểu đồ và publish lệnh LED |

Bài 1 là bài REST độc lập. Bài 2 chỉ publish telemetry. Bài 3 bổ sung chiều điều khiển ngược từ dashboard về ESP32. Bài 4 dùng MQTT Explorer như một client khác để theo dõi và kiểm thử broker/topic; **không cần viết firmware C++ mới**.

## 2. Cấu trúc project

```text
week9/
├── TH4-Bai1-REST-API/
│   ├── include/project_config.h
│   ├── src/main.cpp
│   ├── platformio.ini
│   ├── diagram.json
│   ├── wokwi.toml
│   ├── README.md
│   ├── TH4-Bai1-REST-API.fzz
│   └── TH4-Bai1-REST-API.png
│
├── TH4-Bai2-MQTT-Publisher/
│   ├── include/project_config.h
│   ├── src/main.cpp
│   ├── platformio.ini
│   ├── diagram.json
│   ├── wokwi.toml
│   ├── README.md
│   └── HiveMQ_WebSocket_Client.png
│
├── TH4-Bai3-NodeRED-Dashboard/
│   ├── include/project_config.h
│   ├── src/main.cpp
│   ├── platformio.ini
│   ├── diagram.json
│   ├── wokwi.toml
│   ├── docs/README.md
│   ├── docs/flows/th4-dashboard-flow.json
│   ├── Node-red_config.png
│   ├── Node-red_ui.png
│   ├── Bat_den.png
│   └── Tat_den.png
│
├── TH4-Bai4-MQTT-Explorer/
│   ├── docs/
│   ├── mqtt_config.png
│   ├── Telemetry.png
│   ├── led_on.png
│   └── led_off.png
│
└── esp32-mqtt-platformio-wokwi/   # Ví dụ MQTT mở rộng
```


## 3. Cấu hình dùng chung

### ESP32 và cảm biến

Các sơ đồ Wokwi của Bài 1–3 dùng ESP32 DevKit V1 và DHT22:

| DHT22 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| DATA | GPIO4 |

Bài 3 bổ sung LED rời để điều khiển từ xa:

| Thành phần | Kết nối |
|---|---|
| LED anode | GPIO14 qua điện trở 220 Ω |
| LED cathode | GND |

Bài 1 và Bài 2 không cần LED ngoài trong sơ đồ của project.

### Wi-Fi và MQTT broker

| Thuộc tính | Giá trị đang cấu hình |
|---|---|
| Wi-Fi trên Wokwi | `Wokwi-GUEST` |
| MQTT broker | `broker.hivemq.com` |
| MQTT TCP port | `1883` |
| MQTT protocol | MQTT 3.1.1 |
| Username / password | Để trống với public broker |
| TLS | Tắt trên port 1883 |
| Student ID / namespace | `25112107` |



### MQTT topic contract

```text
aet2022/
└── 25112107/
    ├── telemetry       # ESP32 → broker → client(s), JSON
    └── cmd/
        └── led          # client → broker → ESP32, payload 1 hoặc 0
```

Topic chính xác:

```text
aet2022/25112107/telemetry
aet2022/25112107/cmd/led
```

Payload telemetry ở Bài 2:

```json
{
  "id": "25112107",
  "temp": 24.0,
  "hum": 55.0,
  "uptime": 12,
  "rssi": -45
}
```

Payload telemetry ở Bài 3 có thêm trạng thái LED:

```json
{
  "id": "25112107",
  "temp": 24.0,
  "hum": 55.0,
  "uptime": 12,
  "rssi": -45,
  "led": 0
}
```

Lệnh điều khiển LED dùng payload raw/plain text `1` (bật) hoặc `0` (tắt), QoS 0, Retain tắt. Client ID của ESP32 được tạo từ MAC; MQTT Explorer cần dùng Client ID riêng để tránh đụng nhau.

## 4. Bài 1 – ESP32 REST API

### Mục tiêu và chức năng

ESP32 kết nối Wi-Fi ở chế độ STA, đọc cảm biến DHT22 và cung cấp dữ liệu qua HTTP endpoint:

```http
GET /api/sensor
```

HTTP server chạy trên port `80`. Khi chạy Wokwi, `wokwi.toml` forward cổng máy tính `localhost:8180` đến `target:80`, nên endpoint kiểm thử từ Windows là:

```text
http://localhost:8180/api/sensor
```

### JSON response

Khi cảm biến đọc thành công, server trả HTTP `200` với các trường:

| Field | Ý nghĩa |
|---|---|
| `id` | MSSV/ID đã cấu hình (`25112107`) |
| `rssi` | Cường độ tín hiệu Wi-Fi (dBm) |
| `uptime` | Thời gian ESP32 đã chạy, tính bằng giây |
| `sensor_ok` | Trạng thái đọc cảm biến |
| `temp` | Nhiệt độ (°C) |
| `hum` | Độ ẩm (%) |

Nếu DHT22 lỗi/không đọc được, endpoint trả HTTP `503` cùng JSON báo `sensor_read_failed`, không thay lỗi cảm biến bằng số đo giả. Route không tồn tại trả HTTP `404` với JSON `not_found`.

### Build và chạy

Mở terminal tại thư mục `week9/TH4-Bai1-REST-API`:

```powershell
pio run
```

Khởi động Wokwi trong VS Code, chờ Serial Monitor báo Wi-Fi connected và HTTP server started, rồi kiểm tra:

```powershell
curl.exe http://localhost:8180/api/sensor
```

Hoặc mở URL trên trình duyệt. Đề xuất kiểm thử bổ sung: gọi `/missing` để xác nhận HTTP 404 và kiểm tra trường hợp DHT22 không đọc được để xác nhận HTTP 503.

### Minh chứng hiện có

- [Sơ đồ DHT22 – ESP32 (Fritzing PNG)](./TH4-Bai1-REST-API/TH4-Bai1-REST-API.png)
- [File Fritzing gốc](./TH4-Bai1-REST-API/TH4-Bai1-REST-API.fzz)

**Lưu ý:** GitHub hiện có sơ đồ và source/configuration nhưng chưa có ảnh chụp HTTP JSON response hoặc Serial Monitor cho Bài 1. Cần bổ sung ảnh runtime nếu muốn chứng minh kết quả chạy trực tiếp ngay trong repo.

## 5. Bài 2 – MQTT Publisher

### Mục tiêu và chức năng

ESP32 đọc DHT22 và publish telemetry dạng JSON lên HiveMQ public broker. Firmware dùng PubSubClient, tạo Client ID theo MAC address, duy trì kết nối Wi-Fi/MQTT và gọi `mqtt.loop()` thường xuyên.

- Broker: `broker.hivemq.com:1883`
- Topic: `aet2022/25112107/telemetry`
- Chu kỳ publish: khoảng 2 giây
- Payload: `id`, `temp`, `hum`, `uptime`, `rssi`

Bài 2 **chỉ là Publisher**: firmware không subscribe lệnh LED. Điều khiển LED hai chiều được triển khai ở Bài 3.

### Build và chạy

Mở terminal tại `week9/TH4-Bai2-MQTT-Publisher`:

```powershell
pio run
```

Khởi động Wokwi và theo dõi Serial Monitor ở `115200` baud. Kết nối một MQTT client tới cùng broker, subscribe:

```text
aet2022/25112107/telemetry
```

Có thể dùng HiveMQ WebSocket Client hoặc MQTT Explorer. Mỗi message phải là JSON hợp lệ; `uptime` tăng theo thời gian, còn `temp`/`hum` có thể đứng yên nếu giá trị cảm biến trong Wokwi không đổi.

### Minh chứng hiện có

- [HiveMQ WebSocket Client nhận telemetry](./TH4-Bai2-MQTT-Publisher/HiveMQ_WebSocket_Client.png)
- [Sơ đồ DHT22 – ESP32](./TH4-Bai2-MQTT-Publisher/TH4-Bai2-MQTT-Publisher.png)

Ảnh HiveMQ WebSocket Client cho thấy broker kết nối và nhận các JSON message định kỳ từ topic telemetry.

## 6. Bài 3 – Node-RED Dashboard

### Mục tiêu và chức năng

Bài 3 mở rộng Publisher thành mô hình MQTT hai chiều:

1. ESP32 publish telemetry JSON mỗi khoảng 2 giây.
2. Node-RED subscribe telemetry và phân tích JSON.
3. Dashboard hiển thị nhiệt độ, độ ẩm, biểu đồ lịch sử và uptime.
4. Hai nút LED ON/OFF publish lệnh `1`/`0` lên command topic.
5. ESP32 subscribe command topic, điều khiển LED GPIO14 và thêm trạng thái `led` vào các telemetry tiếp theo.

MQTT contract:

| Hướng | Topic | Payload |
|---|---|---|
| ESP32 → Node-RED | `aet2022/25112107/telemetry` | JSON gồm `id`, `temp`, `hum`, `uptime`, `rssi`, `led` |
| Node-RED → ESP32 | `aet2022/25112107/cmd/led` | `1` hoặc `0` |

### Node-RED Dashboard

Project dùng các node Dashboard classic (`node-red-dashboard`):

- Gauge nhiệt độ.
- Gauge độ ẩm.
- Chart nhiệt độ/độ ẩm theo thời gian.
- Uptime của ESP32.
- Nút LED ON và LED OFF.

Dashboard thường mở tại:

```text
http://localhost:1880/ui
```

Cần cài `node-red-dashboard` trong đúng Node-RED user directory/instance đang chạy, import flow đầy đủ, cấu hình broker `broker.hivemq.com:1883` và Deploy.

### Build firmware

Trong bản project hiện tại trên GitHub, `platformio.ini`, `include/`, `src/`, `diagram.json` và `wokwi.toml` nằm ngay thư mục `TH4-Bai3-NodeRED-Dashboard`:

```powershell
cd week9/TH4-Bai3-NodeRED-Dashboard
pio run
```

Chạy Wokwi và chờ ESP32 kết nối Wi-Fi/MQTT. Node-RED và ESP32 phải dùng cùng broker, cùng Student ID và cùng topic.

### Minh chứng hiện có

- [Cấu hình Node-RED/MQTT](./TH4-Bai3-NodeRED-Dashboard/Node-red_config.png)
- [Dashboard realtime](./TH4-Bai3-NodeRED-Dashboard/Node-red_ui.png)
- [Minh chứng lệnh bật LED](./TH4-Bai3-NodeRED-Dashboard/Bat_den.png)
- [Minh chứng lệnh tắt LED](./TH4-Bai3-NodeRED-Dashboard/Tat_den.png)
- [Sơ đồ Wokwi/Fritzing](./TH4-Bai3-NodeRED-Dashboard/TH4-Bai3-NodeRED-Dashboard.png)

### Repository gap cần xử lý

Ảnh `Node-red_ui.png` chứng minh dashboard đã được dựng; tuy nhiên file flow trên `main` tại `docs/flows/th4-dashboard-flow.json` hiện chỉ có 2 node (`mqtt in` và `mqtt-broker`), chưa chứa các gauge/chart/button như dashboard trong ảnh. Gói `AET2022-TH4-GITHUB-READY.zip` đã lưu trước đó có bản flow đủ 16 node ở `TH4-Bai3-NodeRED-Dashboard/flows/th4-dashboard-flow.json`, cùng cấu trúc firmware trong `device/`. Vì vậy, để clone repo rồi import/deploy lại được, cần đưa flow đầy đủ vào `main` (và đồng bộ lại README/cấu trúc thư mục). Không nên dùng flow rút gọn hiện tại để tuyên bố đã tái tạo dashboard hoàn chỉnh.

## 7. Bài 4 – MQTT Explorer

### Vai trò

Bài 4 là bài quan sát và kiểm thử bằng MQTT Explorer trên Windows. Không có firmware C++ riêng; sử dụng ESP32/Wokwi của Bài 3 đang chạy để publish telemetry và nhận lệnh.

### Connection

| Trường | Giá trị |
|---|---|
| Name | `TH4-25112107` |
| Protocol | `mqtt://` |
| Host | `broker.hivemq.com` |
| Port | `1883` |
| MQTT version | `3.1.1` |
| Username / Password | Để trống |
| TLS / Encryption | Tắt |
| Client ID | Dùng ID riêng, không trùng ESP32 hoặc Node-RED |
| Keep Alive | `60` giây nếu có |

### Subscriptions

Thêm hai subscription QoS 0:

```text
aet2022/25112107/telemetry
aet2022/25112107/cmd/led
```

MQTT Explorer hiển thị cây topic, payload JSON, lịch sử message và JSON Chart. Các trường số nên đưa lên biểu đồ gồm `temp`, `hum`, `uptime`, `rssi`, `led`; không đưa `id` vì đây là chuỗi.

### Publish lệnh LED

Trong panel Publish:

| Trường | Giá trị |
|---|---|
| Topic | `aet2022/25112107/cmd/led` |
| Payload format | Raw / Plain text |
| QoS | `0` |
| Retain | Tắt |

- Gửi `1`: ESP32 nhận lệnh, bật LED GPIO14 và Serial báo `Command received: LED ON`.
- Gửi `0`: ESP32 tắt LED GPIO14 và Serial báo `Command received: LED OFF`.

Giữ Wokwi simulation chạy trong khi kiểm thử. Không bật Retain cho command để tránh phát lại lệnh cũ khi ESP32 subscribe lại.

### Minh chứng hiện có

- [MQTT Explorer connection](./TH4-Bai4-MQTT-Explorer/mqtt_config.png)
- [Telemetry JSON và lịch sử message](./TH4-Bai4-MQTT-Explorer/Telemetry.png)
- [Publish `1` – LED ON](./TH4-Bai4-MQTT-Explorer/led_on.png)
- [Publish `0` – LED OFF](./TH4-Bai4-MQTT-Explorer/led_off.png)

Các ảnh cho thấy MQTT Explorer kết nối HiveMQ, nhận telemetry từ ESP32/Wokwi và publish lệnh điều khiển LED theo topic command.

## 8. Lệnh build tổng hợp

```powershell
# Bài 1 – REST API
cd week9/TH4-Bai1-REST-API
pio run

# Bài 2 – MQTT Publisher
cd week9/TH4-Bai2-MQTT-Publisher
pio run

# Bài 3 – Node-RED Dashboard / firmware ESP32
cd week9/TH4-Bai3-NodeRED-Dashboard
pio run
```

Bài 4 không có lệnh build. Mở MQTT Explorer, kết nối cùng broker và làm các bước subscribe/publish ở mục 7.

## 9. Bonus / project mở rộng

Thư mục [`esp32-mqtt-platformio-wokwi`](./esp32-mqtt-platformio-wokwi) là một ví dụ MQTT riêng, không phải một trong bốn project TH4 chính. Nó mở rộng telemetry với cảm biến áp suất và trạng thái nút:

- Hardware target: AHT20 + BMP280.
- Wokwi target: DHT22 + BMP180 thay thế.
- JSON gồm `temperature`, `humidity`, `pressure`, `button`.
- LED GPIO18, button GPIO4.

Project này dùng namespace MQTT khác (`esp32/sensors`, `esp32/led`) và có cấu hình riêng. Không trộn topic hoặc firmware của project bonus với namespace `aet2022/25112107/...` của bốn bài chính. README của project bonus còn cần rà lại broker/endpoint để đồng bộ với source trước khi dùng làm minh chứng chính thức.