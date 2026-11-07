# TH2 — Timer, Non-blocking và Sleep trên ESP32

**Học phần:** Thí nghiệm Khoa học Kỹ thuật 2 (AET2022)  
**Buổi thực hành:** TH2 — Timer phần cứng, non-blocking và Deep Sleep  
**Sinh viên:** Nguyễn Tuấn Thành  
**MSSV:** 25112107  
**Repository:** [ThanhNguyn/tnkh-kt2](https://github.com/ThanhNguyn/tnkh-kt2)

> TH2 có **4 bài thực hành chính** theo trang 2 của đề. Các thư mục trong `week7/` là project/ví dụ minh họa, không phải số lượng bài. Một số tiêu đề “Bài 2 — Demo” ở các trang sau của slide không khớp với danh sách 4 bài ở trang 2; README này phân loại theo danh sách tổng quan trang 2 và bảng phân biệt Light Sleep (Bài 3)/Deep Sleep (Bài 4) ở trang 15.

## 1. Tổng quan 4 bài theo đề

| Bài | Nội dung yêu cầu | Project trong repository | Đánh giá source hiện có |
|---|---|---|---|
| **1** | LED + cảm biến: so sánh cách viết blocking bằng `delay()` với non-blocking bằng `millis()` | `vd1_dung_delay`, `vd1_non_blocking` | Đã có hai cách triển khai; cần bổ sung minh chứng đo/đồ thị |
| **2** | Deep sleep + `RTC_DATA_ATTR` (trọng tâm); mở rộng đánh thức bằng nút ngoài | `bai2_vd1`, `bai2_vd2` | Đã có deep sleep bằng timer và so sánh RTC với RAM; nút ngoài là phần mở rộng, chưa có source |
| **bonus** | Timer phần cứng (`esp_timer` theo phần tổng quan đề): đo jitter/độ lệch so với `millis()` | `demo_vd1`, `demo_vd2` | Có đo chu kỳ và thống kê Min/Max/Avg; source dùng API `hw_timer_t` của Arduino-ESP32, không gọi trực tiếp `esp_timer_create()` |

**Lưu ý về cách đánh số trong slide:** Trang 2 xác định Bài 3 là so sánh bốn mức sleep và Bài 4 là Deep Sleep + RTC. Trang 15 cũng ghi rõ Light Sleep (Bài 3), Deep Sleep (Bài 4). Trong khi đó trang 13 và 16 lại dùng tiêu đề “Bài 2 — Demo” cho các ví dụ Deep Sleep/so sánh chế độ ngủ. README ưu tiên phân loại theo danh sách chính ở trang 2 và bảng trang 15 để không đánh đồng tiêu đề demo bị lệch số với số bài thực hành.

## 2. Cấu trúc hiện có

```text
week7/
├── vd1_dung_delay/       # Bài 1: phiên bản blocking
├── vd1_non_blocking/     # Bài 1: phiên bản millis()
├── demo_vd1/             # Bài 2: đo chu kỳ bằng millis()
├── demo_vd2/             # Bài 2: đo chu kỳ bằng hardware timer
├── bai2_vd1/             # Nội dung thực tế: Bài 4, RTC_DATA_ATTR + deep sleep timer
└── bai2_vd2/             # Nội dung thực tế: Bài 4, demo wake-up + Wi-Fi/Web Server
```

## 3. Bài 1 — Blocking `delay()` và non-blocking `millis()`

### Ví dụ 1 — `vd1_dung_delay`

Project: [`vd1_dung_delay/`](./vd1_dung_delay) · [Source](./vd1_dung_delay/src/main.cpp)

Code thực hiện lần lượt: LED HIGH 500 ms, LED LOW 500 ms, đọc ADC rồi chờ 2.000 ms, in `heartBeat` rồi chờ 5.000 ms. Tổng thời gian chờ trong một vòng lặp xấp xỉ 8 giây; vì vậy LED không thể đổi trạng thái đều mỗi 500 ms và các tác vụ bị chặn lẫn nhau.

### Ví dụ 2 — `vd1_non_blocking`

Project: [`vd1_non_blocking/`](./vd1_non_blocking) · [Source](./vd1_non_blocking/src/main.cpp)

Code quản lý ba mốc thời gian độc lập (`lanTruocLed`, `lanTruocAdc`, `lanTruocHeart`). `loop()` kiểm tra `millis()` và thực thi từng tác vụ khi đến hạn, không dùng `delay()` trong vòng lặp chính.

### Sơ đồ

- [Fritzing — bản delay (PNG)](./vd1_dung_delay/vd1_dung_delay.png) · [FZZ](./vd1_dung_delay/vd1_dung_delay.fzz)
- [Fritzing — bản non-blocking (PNG)](./vd1_non_blocking/vd1_non_blocking.png) · [FZZ](./vd1_non_blocking/vd1_non_blocking.fzz)

**Tình trạng minh chứng:** đã có source và sơ đồ. Chưa thấy ảnh Serial/đồ thị thực nghiệm hoặc script `plot_serial_th2_bai1.py` trong repository. Source hiện in ADC và heartbeat dạng text thường, chưa xuất format Teleplot `>ten:gia_tri` như gợi ý trong đề.


## 4. Bài 2 — Deep Sleep và `RTC_DATA_ATTR`

### Ví dụ 1 — Giữ biến RTC và làm mất biến RAM

Project: [`bai2_vd1/`](./bai2_vd1) · [Source](./bai2_vd1/src/main.cpp)

- `RTC_DATA_ATTR int soLanThuc = 0;` đặt biến trong RTC memory để giữ giá trị qua deep sleep.
- `int demThuong = 0;` là biến RAM thông thường, được khởi tạo lại sau khi deep sleep làm chip boot lại.
- Mỗi lượt chạy, hai biến tăng lên; Serial in giá trị trước khi ngủ.
- LED GPIO2 tắt, timer wake-up được đặt 10 giây, sau đó gọi `esp_deep_sleep_start()`.

Đây là source trực tiếp thể hiện trọng tâm Bài 4: so sánh dữ liệu RTC với RAM thường qua nhiều lần deep sleep/wake-up.

### Ví dụ 2 — Wake-up, Wi-Fi và Web Server

Project: [`bai2_vd2/`](./bai2_vd2) · [Source](./bai2_vd2/src/main.cpp)

- In nguyên nhân wake-up bằng `esp_sleep_get_wakeup_cause()` và reset reason bằng `esp_reset_reason()`.
- Nhánh hardware: khi power-on, cấu hình timer wake-up 5 giây và deep sleep; sau timer wake-up, chương trình khởi tạo mạng và Web Server.
- Nhánh Wokwi hiện đặt `WOKWI_SIMULATION 1`: kết nối `Wokwi-GUEST`, bỏ qua lệnh Deep Sleep, chạy Web Server và LED heartbeat. Vì vậy Wokwi chỉ chứng minh phần web/heartbeat, không chứng minh deep sleep thực sự.

### Mở rộng — đánh thức bằng nút ngoài

Đề ghi đây là phần mở rộng. Source hiện chưa có `esp_sleep_enable_ext0_wakeup()`/`esp_sleep_enable_ext1_wakeup()` và sơ đồ chưa thể hiện nút nối RTC GPIO. Đây là bonus chưa triển khai, không nên đánh đồng với yêu cầu trọng tâm Deep Sleep + RTC đã có trong `bai2_vd1`.

### Sơ đồ

- [Fritzing — RTC/deep sleep (PNG)](./bai2_vd1/bai2_vd1.png) · [FZZ](./bai2_vd1/bai2_vd1.fzz)
- [Fritzing — Web Server/deep sleep (PNG)](./bai2_vd2/bai2_vd2.png) · [FZZ](./bai2_vd2/bai2_vd2.fzz)

**Tình trạng minh chứng:** source trọng tâm đã có, nhưng repo hiện lưu sơ đồ mạch chứ chưa có ảnh Serial chứng minh `soLanThuc` được giữ và `demThuong` reset sau nhiều chu kỳ wake-up. Cần bổ sung log/ảnh nếu yêu cầu nộp minh chứng đầy đủ.

## 5. Bài demo — Timer phần cứng và jitter

### Ví dụ 1 — `millis()`

Project: [`demo_vd1/`](./demo_vd1) · [Source](./demo_vd1/src/main.cpp)

- Chu kỳ đặt: `timerMs = 20` ms.
- `updateLoopTimer()` kiểm tra lịch trong `loop()`.
- Khi tới hạn, lấy `micros()` để đo khoảng cách giữa hai lần thực thi.
- Tích lũy `dtMin`, `dtMax`, `dtSum`, `count`; in Min/Max/Avg khoảng mỗi 2 giây.

### Ví dụ 2 — Hardware timer

Project: [`demo_vd2/`](./demo_vd2) · [Source](./demo_vd2/src/main.cpp)

- Khởi tạo `hw_timer_t` với độ phân giải 1 MHz qua `timerBegin(1000000)`.
- Đặt alarm lặp mỗi 20 ms bằng `timerAlarm(...)`.
- ISR lấy timestamp `micros()` và ghi nhận khoảng thời gian; dùng critical section để bảo vệ dữ liệu chia sẻ.
- `loop()` đọc bản chụp thống kê và in Min/Max/Avg.

**Ghi chú API:** phần tổng quan trang 2 ghi `esp_timer`; ví dụ minh họa timer trong các slide dùng API `hw_timer_t` của Arduino-ESP32. Source hiện tại đi theo API `hw_timer_t` (Arduino-ESP32 Core 3.x), không gọi trực tiếp `esp_timer_create()` của ESP-IDF. Đây là điểm cần giữ chính xác khi thuyết minh.

### Sơ đồ

- [Fritzing — millis (PNG)](./demo_vd1/demo_vd1.png) · [FZZ](./demo_vd1/demo_vd1.fzz)
- [Fritzing — hardware timer (PNG)](./demo_vd2/demo_vd2.png) · [FZZ](./demo_vd2/demo_vd2.fzz)

**Tình trạng minh chứng:** đã có hai implementation thống kê khoảng thời gian Min/Max/Avg. Chưa thấy ảnh Serial/đồ thị kết quả lưu trong repo và source chưa tạo tải CPU có chủ đích; do đó so sánh trong điều kiện “CPU bận” chưa được chứng minh đầy đủ bằng artifacts hiện có.

## 7. Các phần mở rộng hiện có

- Hai demo định kỳ riêng bằng `millis()` và `hw_timer_t`, có ghi Min/Max/Avg cho chu kỳ 20 ms.
- Demo Deep Sleep bổ sung Wi-Fi và HTTP Web Server sau wake-up (`bai2_vd2`).
- LED GPIO2 heartbeat để quan sát chương trình hoạt động sau khi mạng/Web Server khởi tạo.
- Mỗi project có sơ đồ Fritzing và cấu hình Wokwi.

Các phần mở rộng này không thay thế Bài 3 (so sánh bốn mức ngủ). External-button wake-up là một mở rộng khác của Bài 4 và hiện chưa có source.

## 8. Build và chạy project

Các project dùng PlatformIO, Arduino framework, target `esp32dev`, Serial Monitor 115200 baud. Chạy từng lệnh trong thư mục tương ứng.

### Bài 1

```powershell
cd week7/vd1_dung_delay
pio run -e esp32dev
```

```powershell
cd week7/vd1_non_blocking
pio run -e esp32dev
```

### Bài 2

```powershell
cd week7/bai2_vd1
pio run -e esp32dev
```

```powershell
cd week7/bai2_vd2
pio run -e esp32dev
```

### Bài demo

```powershell
cd week7/demo_vd1
pio run -e esp32dev
```

```powershell
cd week7/demo_vd2
pio run -e esp32dev
```