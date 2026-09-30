# TNKH-KT2 — ESP32 / IoT Coursework

![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange?logo=platformio)
![Wokwi](https://img.shields.io/badge/Simulator-Wokwi-6F4CFF)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![MQTT](https://img.shields.io/badge/MQTT-HiveMQ-6600CC)

Repository lưu các bài thực hành ESP32/IoT theo từng tuần, sử dụng PlatformIO, VS Code và Wokwi. Nội dung đi từ GPIO cơ bản đến interrupt, PWM, timer, ADC, OLED/DHT, Web Server, Access Point, REST/JSON, MQTT và Node-RED.

> Mỗi thư mục là một project độc lập, có thể mở trực tiếp bằng VS Code + PlatformIO.

## 📚 Nội dung

| Tuần | Nội dung | Thư mục |
|---|---|---|
| Week 1 | GPIO, LED, Button, ADC | [week1](./week1) |
| Week 2 | PlatformIO/Wokwi, interrupt, PWM, timer, ADC, OLED + DHT | [week2](./week2) |
| Week 3 | Web Server, DHT sensor, AP mode | [week3](./week3) |
| Week 4 | REST JSON, MQTT, Node-RED dashboard, LED control | [week4](./week4) |

---

# Week 1 — GPIO cơ bản

- [ESP32_Blink_LED](./week1/ESP32_Blink_LED) — LED blink bằng `delay()`.
- [ESP32_Blink_Millis](./week1/ESP32_Blink_Millis) — LED blink không blocking bằng `millis()`.
- [ESP32_Button_LED](./week1/ESP32_Button_LED) — button điều khiển LED với `INPUT_PULLUP`.
- [ESP32_ADC_Potentiometer](./week1/ESP32_ADC_Potentiometer) — đọc ADC trên GPIO34.
- [esp32-led-button](./week1/esp32-led-button) — button + LED + Serial log.

### Kỹ năng

Digital I/O, `INPUT_PULLUP`, `millis()`, ADC ESP32, Serial Monitor.

---

# Week 2 — PlatformIO, Wokwi và peripheral nâng cao

- [ESP32_PlatformIO_Wokwi_Setup](./week2/ESP32_PlatformIO_Wokwi_Setup)
- [ESP32_Lab1_Bac4_OLED_DHT11](./week2/ESP32_Lab1_Bac4_OLED_DHT11)
- [demo1](./week2/demo1) — external interrupt + debounce.
- [demo2](./week2/demo2) — LEDC PWM + hardware timer + ADC.
- [demo3](./week2/demo3) — ADC raw/theoretical/calibrated comparison.
- [demo4](./week2/demo4) — GPIO12 strapping-pin demonstration.
- [ghep-demo1-demo2](./week2/ghep-demo1-demo2) — kết hợp interrupt + PWM + hardware timer.
- [led-sang-dan](./week2/led-sang-dan) — button thay đổi độ sáng LED bằng PWM.

### Lab 1

`ESP32_Lab1_Bac4_OLED_DHT11` sử dụng:

- Button: GPIO18
- LED/PWM: GPIO4
- DHT: GPIO27
- OLED I2C: SDA GPIO21, SCL GPIO22
- OLED address: `0x3C`
- Serial: `115200`
- Debounce: `50 ms`
- DHT interval: `2 s`

Wokwi mô phỏng DHT22; source có ghi chú khi dùng kit thật của lab có thể đổi sang DHT11.

---

# Week 3 — ESP32 Web Server

## Bài 1 — Web Server + LED

[week3/bai1-web-server](./week3/bai1-web-server)

ESP32 chạy Web Server port 80 với:

- `/` — giao diện web
- `/on` — bật LED
- `/off` — tắt LED
- `/toggle` — đảo trạng thái LED

LED dùng GPIO2.

## Bài 2 — DHT Web Server

[week3/bai2-dht11-web-server](./week3/bai2-dht11-web-server)

Bổ sung:

- DHT sensor
- Web UI nhiệt độ/độ ẩm
- `/sensor` trả JSON
- `/on`, `/off`, `/toggle`
- JavaScript `fetch()` cập nhật sensor định kỳ

## Bài 3 — Access Point Mode

[week3/bai3-ap-mode](./week3/bai3-ap-mode)

ESP32 tạo Access Point:

- SSID: `ThanhESP32`
- Password: `Thanh@2026`
- AP IP: `192.168.4.1`

Web UI hỗ trợ ON/OFF/TOGGLE LED. Project cũng có nhánh cấu hình mô phỏng Wokwi với `WOKWI_SIM`.

---

# Week 4 — REST JSON + MQTT + Node-RED

Project:

[week4/esp32_mqtt_node_red](./week4/esp32_mqtt_node_red)

## Yêu cầu

ESP32 thực hiện đồng thời:

1. Trả nhiệt độ/độ ẩm dưới dạng JSON tại `GET /api/sensor`.
2. Publish nhiệt độ/độ ẩm qua MQTT lên dashboard realtime bằng Node-RED.

### Hardware

| Thiết bị | GPIO |
|---|---:|
| DHT22 DATA | GPIO4 |
| LED | GPIO2 |

### Wi-Fi — Wokwi

```text
SSID: Wokwi-GUEST
Password: để trống
Channel: 6
```

### MQTT

```text
Broker: broker.hivemq.com
Port: 1883

Sensor:
tnkh/week4/esp32/sensor

LED:
tnkh/week4/esp32/led
```

Payload sensor mẫu:

```json
{
  "temperature": 24,
  "humidity": 55
}
```

LED command: `ON` / `OFF`.

### REST API

```text
GET /api/sensor
```

Response khi sensor hợp lệ:

```json
{
  "temperature": 24,
  "humidity": 55,
  "unit_temperature": "C",
  "unit_humidity": "%",
  "mqtt_topic": "tnkh/week4/esp32/sensor"
}
```

Firmware không giả giá trị `0` khi DHT22 chưa sẵn sàng; trường hợp chưa có dữ liệu hợp lệ trả HTTP `503`.

### Node-RED Dashboard

Node-RED subscribe:

```text
tnkh/week4/esp32/sensor
```

Dashboard gồm:

- Temperature chart
- Temperature gauge
- Humidity chart
- Humidity gauge

Dashboard:

```text
http://localhost:1880/ui
```

### Phần mở rộng

Hai nút:

- LED ON
- LED OFF

gửi lệnh qua:

```text
tnkh/week4/esp32/led
```

ESP32 nhận lệnh và điều khiển LED GPIO2.

### Minh chứng

- Ảnh browser hiển thị JSON từ `/api/sensor`.
- Ảnh dashboard hiển thị biểu đồ realtime.
- Nếu làm phần mở rộng: minh chứng LED ON/OFF qua MQTT.

---

# 🛠️ Công cụ

- VS Code
- PlatformIO
- Wokwi for VS Code
- ESP32 DevKit
- Node-RED
- MQTT / HiveMQ
- DHT11 / DHT22
- OLED SSD1306
- C++ / Arduino framework

---

# 🚀 Cách chạy project

Mẫu cấu trúc:

```text
project/
├── platformio.ini
├── wokwi.toml
├── diagram.json
├── src/
│   └── main.cpp
└── ...
```

Build:

```bash
pio run
```

Với Wokwi:

1. Mở đúng project.
2. Build firmware.
3. Mở `diagram.json`.
4. Start Wokwi Simulator.
5. Kiểm tra Serial Monitor ở `115200` nếu project có Serial output.

Với Week 4, chạy Node-RED rồi import `node-red-flow.json`, sau đó **Deploy** trước khi mở dashboard.

---

# 🧪 Checklist kiểm tra code

- [ ] Pin trong `main.cpp` khớp `diagram.json`.
- [ ] Dependencies trong `platformio.ini` khớp code.
- [ ] `wokwi.toml` trỏ đúng firmware/ELF.
- [ ] Serial baud đúng `115200`.
- [ ] DHT đọc theo chu kỳ phù hợp.
- [ ] Không trả dữ liệu `0` giả khi sensor chưa sẵn sàng.
- [ ] MQTT publish/subscribe dùng đúng topic.
- [ ] Payload MQTT là JSON hợp lệ.
- [ ] Node-RED parse JSON trước khi đưa dữ liệu số vào chart/gauge.
- [ ] HTTP endpoint trả đúng `Content-Type`.
- [ ] Dashboard không bị duplicate flow/tab do import nhiều lần.

---

# 📝 Week 4 — Câu hỏi ôn tập

1. Vì sao app điện thoại nên nhận JSON từ ESP32 thay vì nhận HTML?
2. Kể ba phương thức HTTP của REST và công dụng của chúng trên ESP32.
3. Mô tả publisher, subscriber, broker và topic trong MQTT.
4. Khi nào MQTT phù hợp hơn việc gọi REST liên tục?
5. So sánh Node-RED, Blynk/Adafruit IO và ThingsBoard trong một đồ án nhỏ.
6. WiFiManager, Preferences và OTA giải quyết vấn đề gì khi biến prototype thành sản phẩm?

## Tình huống

1. App điện thoại muốn lấy nhiệt độ từ ESP32 — nên nhận HTML hay dữ liệu gọn?
2. 20 cảm biến gửi dữ liệu liên tục — có nên để màn hình hỏi từng cảm biến mỗi giây?
3. Muốn xem biểu đồ nhiệt độ và điều khiển đèn từ xa — LAN có đáp ứng được không?
4. Thiết bị đã lắp cố định và firmware có lỗi — có nhất thiết phải tháo xuống để nạp lại không?

---

## License

Repository phục vụ mục đích học tập và thực hành cá nhân.
