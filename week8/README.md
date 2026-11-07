# TH3 — Kết nối không dây trên ESP32

**Học phần:** Thí nghiệm Khoa học Kỹ thuật 2 (AET2022)  
**Bài thực hành:** TH3 — Tầng mạng: kết nối không dây trên ESP32  
**Nhóm:** 02  
**Sinh viên:** Nguyễn Tuấn Thành  
**MSSV:** 25112107  
**Repository:** [ThanhNguyn/tnkh-kt2](https://github.com/ThanhNguyn/tnkh-kt2)

> Phạm vi kiểm thử: các kết quả trong repository được thực hiện bằng mô phỏng phần mềm. Chưa có kiểm thử trên kit ESP32 vật lý. Môi trường và giới hạn của từng bài được ghi riêng bên dưới.

## 1. Tổng quan

Bài thực hành gồm năm nội dung:

| Bài | Nội dung | Môi trường kiểm thử chính | Kết quả |
|---|---|---|---|
| 1 | Wi-Fi Access Point (AP) và Station (STA) | ESP-EMU / Espressif | AP phát SSID; STA kết nối và nhận IP |
| 2 | BLE GATT Server | ESP-EMU + Bumble | Quét, kết nối, đọc và ghi characteristic |
| 3 | ESP-NOW | Breadboard Build | Hai ESP32 ảo truyền/nhận struct trực tiếp |
| 4 | HTTP Web Server | Wokwi | Trình duyệt điều khiển LED qua HTTP |
| 5 | MQTT Publish/Subscribe | Wokwi + HiveMQ + MQTT Explorer | Publish counter và điều khiển LED qua topic |

Các demo Breadboard bổ sung được ghi là **bonus**, không thay thế kết quả chính của Bài 1 và Bài 2.

## 2. Cấu trúc repository

```text
week8/
├── bai1_wifi/
│   ├── include/wifi_config.h
│   ├── src/
│   │   ├── ap/main.cpp
│   │   ├── sta/main.cpp
│   │   └── apsta/main.cpp
│   ├── scripts/merge_emulator_image.py
│   ├── platformio.ini
│   ├── README.md
│   ├── 01-ap-success.png.png
│   ├── 02-sta-success.png
│   ├── 03-apsta-success.png
│   ├── bonus_breadboard_AP.png
│   └── bonus_breadboard_STA.png
├── bai2_ble/
│   ├── src/main.cpp
│   ├── tools/ble_gatt_client.py
│   ├── B2_BLE.breadboard.json
│   ├── platformio.ini
│   ├── README.md
│   ├── server_and_client_demo.png
│   ├── bonus_server_breadboard.png
│   └── bonus_client_breadboard.png
├── bai3_espnow/
│   ├── src/transmitter.cpp
│   ├── src/receiver.cpp
│   ├── B3_espnow.breadboard.json
│   ├── platformio.ini
│   ├── transmitter.png
│   └── receiver.png
├── bai4_http/
│   ├── src/main.cpp
│   ├── diagram.json
│   ├── bai4_http.fzz
│   ├── bai4_http.png
│   ├── wokwi.toml
│   ├── platformio.ini
│   ├── Led_on.png
│   └── Led_off.png
└── bai5_mqtt/
    ├── src/main.cpp
    ├── diagram.json
    ├── wokwi.toml
    ├── platformio.ini
    ├── mqtt_exploxer.png
    ├── mqtt_led_on.png
    ├── mqtt_led_off.png
    └── wokwi_demo.png
```

## 3. Bài 1 — Wi-Fi Access Point và Station

### Mục tiêu

- ESP32 chạy chế độ **AP** để tự tạo mạng Wi-Fi.
- ESP32 thứ hai chạy chế độ **STA** để kết nối vào AP (hoặc router Wi-Fi khác) và nhận địa chỉ IP.
- Quan sát SSID, IP, MAC và trạng thái kết nối qua Serial Monitor.

### Triển khai

Source được chia thành các phần AP và STA riêng trong `src/ap/` và `src/sta/`. Cấu hình Wi-Fi nằm trong `include/wifi_config.h`.

- AP gọi `WiFi.softAP()` và in SSID, địa chỉ IP AP, MAC và số station đang kết nối.
- STA gọi `WiFi.begin()`, chờ trạng thái `WL_CONNECTED`, sau đó in SSID, IP, gateway, subnet và RSSI.
- Phần APSTA là phần mở rộng để thử AP và STA đồng thời.

**Cấu hình trong lượt kiểm thử:** `MyESP32AP` / `password123`; AP mặc định hiển thị `192.168.4.1`. Cần thay SSID/password theo quy định tên nhóm của lớp trước khi dùng trên kit thật nếu giảng viên yêu cầu SSID riêng.

### Kết quả

- [Ảnh AP trên ESP-EMU](bai1_wifi/01-ap-success.png.png)
- [Ảnh STA kết nối AP trên ESP-EMU](bai1_wifi/02-sta-success.png)
- [Ảnh demo APSTA bổ sung](bai1_wifi/03-apsta-success.png)
- [Bonus Breadboard — AP](bai1_wifi/bonus_breadboard_AP.png)
- [Bonus Breadboard — STA](bai1_wifi/bonus_breadboard_STA.png)

**Lưu ý về bonus Breadboard:** AP và STA đã được kiểm tra riêng. Ở lượt STA bonus, board kết nối vào mạng mô phỏng `BreadboardNet`, không phải SoftAP `MyESP32AP` do board AP tạo. Vì vậy ảnh bonus này chỉ chứng minh thêm khả năng chạy AP và STA trong simulator, không được mô tả là kiểm thử AP-to-STA trực tiếp giữa hai board Breadboard.

## 4. Bài 2 — BLE GATT Server

### Mục tiêu

ESP32 đóng vai trò BLE GATT Server, quảng bá thiết bị và cung cấp một characteristic cho client đọc/ghi.

### Cấu hình GATT

| Thành phần | Giá trị |
|---|---|
| Device name | `TH3-BLE` |
| Service UUID | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` |
| Characteristic UUID | `beb5483e-36e1-4688-b7f5-ea07361b26a8` |
| Giá trị ban đầu | `TH3 BLE Server Ready` |
| Quyền characteristic | `READ` và `WRITE` |

Firmware C++ sử dụng **NimBLE-Arduino 2.5.2**. Callback ghi nhận sự kiện client kết nối/ngắt kết nối, đọc và ghi characteristic; khi client ngắt kết nối, BLE advertising được khởi động lại.

### Kiểm thử

- Demo chính: ESP-EMU chạy BLE server C++; Bumble chạy như BLE GATT client trên máy host để scan, connect, discover service/characteristic, đọc giá trị ban đầu, ghi dữ liệu và đọc lại xác nhận.
- Demo bonus: Breadboard Build sử dụng hai ESP32-C5 chạy MicroPython trong môi trường mô phỏng để minh họa client/server BLE. Kết quả hiển thị kết nối GATT, đọc `TH3 BLE Server Ready`, ghi `25112107` và đọc lại đúng giá trị.

### Kết quả

- [Ảnh demo BLE server/client trên ESP-EMU](bai2_ble/server_and_client_demo.png)
- [Ảnh Breadboard BLE client](bai2_ble/bonus_client_breadboard.png)
- [Ảnh Breadboard BLE server](bai2_ble/bonus_server_breadboard.png)

**Lưu ý về khả năng tái chạy:** `B2_BLE.breadboard.json` lưu bố cục hai board ESP32-C5, nhưng các đoạn MicroPython chạy trong REPL chưa được lưu thành hai file `.py` riêng trong repository. Các ảnh trên là minh chứng cho lượt demo đã chạy; cần bổ sung script nếu muốn tái tạo bonus Breadboard từ source một cách đầy đủ.

## 5. Bài 3 — ESP-NOW

### Mục tiêu

Hai ESP32 giao tiếp trực tiếp bằng ESP-NOW, không cần router Wi-Fi. Một board làm Transmitter và board còn lại làm Receiver.

### Triển khai

- Cả hai board khởi tạo Wi-Fi interface ở chế độ STA.
- Cả hai đặt Wi-Fi channel là `1`.
- Transmitter gửi struct `GoiTin { int dem; }` đến MAC Receiver mỗi 2 giây.
- Receiver đăng ký callback nhận dữ liệu, kiểm tra kích thước gói, sao chép struct và in MAC nguồn cùng counter.

### Kết quả Breadboard Build

| Vai trò | MAC trong lượt mô phỏng |
|---|---|
| Receiver | `7C:DF:00:43:B2:C1` |
| Transmitter | `7C:DF:00:43:B2:C2` |

Receiver đã in các counter tăng dần (`1`, `2`, `3`...), xác nhận nhận được gói từ Transmitter.

- [Ảnh Serial Transmitter](bai3_espnow/transmitter.png)
- [Ảnh Serial Receiver](bai3_espnow/receiver.png)
- [Bố cục Breadboard](bai3_espnow/B3_espnow.breadboard.json)

**Lưu ý:** hai địa chỉ MAC trên là MAC của lượt mô phỏng Breadboard, không phải MAC để sử dụng trên kit thật. Khi chạy hardware, cần đọc MAC Receiver thực tế rồi cập nhật `receiverMac` trong `src/transmitter.cpp`. Cần giữ hai board trên cùng channel. Thử nghiệm chéo với nhóm khác chưa được ghi nhận trong bộ minh chứng hiện tại.

## 6. Bài 4 — HTTP Web Server điều khiển LED

### Mục tiêu

ESP32 kết nối Wi-Fi ở chế độ STA, chạy HTTP Web Server tại port `80` và cung cấp trang web để điều khiển LED GPIO2.

### Chức năng

| HTTP route | Chức năng |
|---|---|
| `/` | Hiển thị giao diện và trạng thái LED |
| `/on` | Bật LED |
| `/off` | Tắt LED |
| `/toggle` | Đảo trạng thái LED |

LED ngoài được mắc với GPIO2 qua điện trở 220 Ω và GND. Source sử dụng `WebServer`, đăng ký các route và gọi `server.handleClient()` trong `loop()`.

### Mô phỏng và kết quả

Bài được chạy trên Wokwi. `wokwi.toml` chuyển tiếp cổng `localhost:8180` trên máy tính đến cổng `80` của ESP32 mô phỏng.

- [Sơ đồ mạch Fritzing PNG](bai4_http/bai4_http.png)
- [File Fritzing gốc](bai4_http/bai4_http.fzz)
- [Ảnh trạng thái LED bật](bai4_http/Led_on.png)
- [Ảnh trạng thái LED tắt](bai4_http/Led_off.png)

**Giới hạn kiểm thử:** trang web được mở qua Wokwi Gateway trên máy tính (`http://localhost:8180/`). Chưa kiểm thử truy cập từ điện thoại vào ESP32 vật lý trên cùng mạng Wi-Fi như kịch bản hardware của đề.

## 7. Bài 5 — MQTT Publish/Subscribe

### Mục tiêu

ESP32 kết nối Wi-Fi, kết nối MQTT broker, publish bộ đếm định kỳ và subscribe topic điều khiển LED.

### Cấu hình

| Thành phần | Giá trị |
|---|---|
| Wi-Fi simulator | `Wokwi-GUEST` |
| MQTT broker | `broker.hivemq.com` |
| TCP port | `1883` |
| Counter topic | `bcse/th3/nhom-02` |
| LED command topic | `bcse/th3/nhom-02/led` |
| MQTT client library | PubSubClient 2.8 |
| LED pin | GPIO2 |

### Luồng hoạt động

1. ESP32 kết nối Wi-Fi `Wokwi-GUEST`.
2. ESP32 kết nối HiveMQ public broker và subscribe `bcse/th3/nhom-02/led`.
3. Cứ mỗi 2 giây, ESP32 publish counter tăng dần lên `bcse/th3/nhom-02`.
4. MQTT Explorer trên Windows subscribe counter topic để xem dữ liệu.
5. MQTT Explorer publish payload `1` hoặc `0` đến topic LED; ESP32 nhận lệnh và bật/tắt LED tương ứng.

### Kết quả

- [Ảnh MQTT Explorer nhận counter](bai5_mqtt/mqtt_exploxer.png)
- [Ảnh publish lệnh bật LED](bai5_mqtt/mqtt_led_on.png)
- [Ảnh publish lệnh tắt LED](bai5_mqtt/mqtt_led_off.png)
- [Ảnh tổng thể Wokwi demo](bai5_mqtt/wokwi_demo.png)

**Lưu ý:** đây là kiểm thử một ESP32 mô phỏng giao tiếp với MQTT Explorer qua broker công cộng; MQTT Explorer đóng vai trò MQTT client thứ hai. Chưa phải kiểm thử trao đổi giữa hai kit ESP32 vật lý. Broker công cộng không phù hợp để gửi dữ liệu nhạy cảm và có thể thay đổi trạng thái dịch vụ.

## 8. Lệnh build nhanh

Chạy lệnh trong thư mục tương ứng của từng bài. Cần cài PlatformIO và các dependency được khai báo trong `platformio.ini`.

### Bài 1 — Wi-Fi

```powershell
cd week8/bai1_wifi
pio run -e emu_ap
pio run -e emu_sta
```

Build bonus Breadboard STA:

```powershell
pio run -e breadboard_sta
```

Các environment `classic_ap` và `classic_sta` dành cho target ESP32 classic. Cần chọn đúng firmware/target khi nạp vào simulator tương ứng.

### Bài 2 — BLE

```powershell
cd week8/bai2_ble
pio run -e emu_ble
```

Bumble client nằm tại `tools/ble_gatt_client.py`; cần chạy trong môi trường Python đã cài Bumble và kết nối với ESP-EMU BLE virtual controller theo hướng dẫn của script.

### Bài 3 — ESP-NOW

```powershell
cd week8/bai3_espnow
pio run -e receiver
pio run -e transmitter
```

### Bài 4 — HTTP

```powershell
cd week8/bai4_http
pio run -e wokwi
```

Sau khi chạy Wokwi và bật gateway, mở `http://localhost:8180/` trên trình duyệt máy tính.

### Bài 5 — MQTT

```powershell
cd week8/bai5_mqtt
pio run -e esp32dev
```

Chạy Wokwi, sau đó kết nối MQTT Explorer đến `broker.hivemq.com:1883`, subscribe counter topic và publish `1`/`0` tới LED command topic.

## 9. Giới hạn và lưu ý khi chuyển sang hardware thật

- **Bài 1:** thay SSID/password theo mạng/AP thực tế; nếu dùng AP của ESP32, STA phải nhập đúng SSID/password của AP. Breadboard bonus dùng `BreadboardNet` chỉ dành cho simulator.
- **Bài 2:** firmware hiện build cho ESP32-S3. Khi đổi sang board khác, cần chọn đúng PlatformIO board tương ứng và kiểm tra hỗ trợ BLE/NimBLE. Trên hardware có thể dùng nRF Connect theo yêu cầu đề.
- **Bài 3:** phải lấy MAC Receiver thật, cập nhật `receiverMac` trong Transmitter và đảm bảo hai board cùng channel.
- **Bài 4:** đặt SSID/password thật trong source; dùng điện thoại cùng mạng Wi-Fi truy cập `http://<IP-ESP32>/`. Không dùng `localhost:8180` trên hardware.
- **Bài 5:** dùng Wi-Fi có Internet, thay placeholder SSID/password trong nhánh hardware; giữ broker/topic nếu broker vẫn hoạt động và mạng cho phép TCP port 1883.
- Chưa có kiểm thử phần cứng vật lý cho năm bài trong phiên bản này.

## 10. Kết luận

Đã xây dựng và kiểm thử mô phỏng năm nội dung của TH3: Wi-Fi AP/STA, BLE GATT, ESP-NOW, HTTP Web Server và MQTT Publish/Subscribe. Các kết quả, cấu hình và ảnh minh chứng được lưu theo từng thư mục bài. Các mục bonus và giới hạn của từng simulator được phân biệt rõ để không nhầm kết quả mô phỏng với kiểm thử trên kit ESP32 thật.
