# Week 5 — TH1: GPIO nâng cao, ngắt ngoài và chân cấm trên ESP32

Nguồn đề: `TH1-gpio-ngat-chan-cam.pdf` (28 trang, 4 tiết). Source key thầy: `TH1-source-code/` (đã note vào đây rồi xóa).

## Ôn nhanh (trang 3)

- ESP32 3.3V, không đưa 5V vào GPIO.
- Strapping GPIO 0,2,12,15: không để thiết bị ngoài kéo lúc cấp nguồn, sai là không boot.
- Input-only 34,35,36,39: chỉ đọc, không `digitalWrite`, không pull nội.
- ADC2 (0,2,4,12-15,25-27) không đọc được khi Wi-Fi bật → dùng ADC1 (32-39).

## Yêu cầu nộp chung (trang 26, trong 24h, cá nhân)

- Project PlatformIO đúng `src/main.cpp` + `platformio.ini` (1 hoặc 3 project).
- Fritzing (`.fzz`/ảnh export, bắt buộc, không vẽ tay): 1 bản Bài 1+2, 1 bản Bài 3. Mỗi sơ đồ thấy rõ ESP32 + linh kiện + dây đúng chân trong code.

## Bài 1 — LED fading: polling vs ngắt ngoài

- VD1 polling: 2 vòng `for` fade 0→255→0, mỗi bước `delay(10)` (~2s). Nút chỉ check 1 lần sau 2 vòng → phản hồi trễ.
- VD2 ngắt: `attachInterrupt(BUTTON FALLING)` → ISR chỉ `buttonPressed=true` → `loop()` chớp LED báo ngay.
- Chân: nút GPIO4 `INPUT_PULLUP`, LED GPIO2 LEDC 5kHz 8-bit.
- Nộp: sơ đồ chung + video LED khi nhấn nút ở cả 2 bản để so sánh.

## Bài 2 — Debounce

- Nút cơ nảy 5–20ms → 1 nhấn sinh nhiều sườn.
- Thầy 2 bản: `vd1_no_debounce` (ISR `isrCount++` thô, stream `>pinState/>isrCount` 500Hz) vs `vd2_debounce` (ISR `millis()` cửa sổ 50ms, stream `>pinState/>rawEdgeCount/>debouncedCount`).
- Thử 3 ngưỡng vd 5/50/500ms, chốt 1 ngưỡng + 1 dòng giải thích. Dùng chung sơ đồ Bài 1.
- Nộp: ngưỡng đã chọn + giải thích + KQ nhấn nhanh 5 lần (ảnh `plot_serial_vd2.py`: raw tăng nhiều, deb chỉ +1).
- Tool vẽ: `pip install pyserial matplotlib` → `python plot_serial_vd2.py COM7`, hoặc Teleplot đọc `>ten:gia_tri`.

## Bài 3 — ADC + lọc + LEDC

- Chuẩn: pot → GPIO34 (ADC1_CH6) + LED → GPIO18 (LEDC 5kHz 8-bit). Đọc thô → lọc trung bình trượt N=8 → `map(0-4095→0-255)`.
- Chạy bản thô chụp số nhảy loạn dù không chạm pot, thêm lọc chụp lại so sánh (`>tho` đỏ vs `>loc` xanh, `plot_serial_vd3.py`).
- Bản lỗi: pot → GPIO12 (ADC2 + strapping MTDI) + bật Wi-Fi sau 3s (`WiFi.begin` không cần nối được) → boot lỗi / đọc sai-hang (`>wifiBat:1`).
- Nộp: sơ đồ Fritzing Bài 3 + ảnh/Plotter thô vs lọc + đánh giá ADC2.
- Lưu ý code: pre-fill buffer (`bufDaDay`), không chia trung bình trên mảng toàn 0 lúc khởi động.

## Map project trong folder này

- [bai1a_khong_ngat](./bai1a_khong_ngat) — = VD1 polling.
- [bai1b_co_ngat](./bai1b_co_ngat) — = VD2 ngắt (nên nháy 3 lần như key thầy để quay video rõ).
- [bai2_debounce](./bai2_debounce) — = vd2 (thiếu vd1_no_debounce, copy thêm từ key).
- [bai3a_adc1](./bai3a_adc1) — = vd1 ADC1 (fix `bufDaDay`, `USE_FILTER/SIM_NOISE` để chụp 2 bản).
- [bai3b_adc2](./bai3b_adc2) — = vd2 ADC2 (m đang GPIO15, key thầy GPIO12 kinh điển hơn; thêm delay 3s bật Wi-Fi + `>wifiBat`).
- Script vẽ: copy `plot_serial.py`, `plot_serial_vd2.py`, `plot_serial_vd3.py` từ key thầy về đây.

## Ôn tập (trang 27)

1. Vì sao ISR ngắn, không `Serial.print`/`delay()`?
2. Debounce giải quyết gì? Ngưỡng quá lớn lỗi gì?
3. Vì sao chọn ADC1 thay vì ADC2?
4. LEDC khác `analogWrite()` ở điểm nào?
