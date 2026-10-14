# Week 5 (TH1) — Đề bài code

## Bài 1a: LED fading polling
- 2 vòng `for` fade 0→255→0, mỗi bước `pwm + delay(10)`, check nút GPIO4 trong vòng lặp → phản hồi trễ.
- Nút GPIO4 `INPUT_PULLUP`, LED GPIO2 LEDC 5kHz 8-bit.
- Project: [bai1a_khong_ngat](./bai1a_khong_ngat)

## Bài 1b: LED fading + ngắt ngoài
- `attachInterrupt(GPIO4, FALLING)` → ISR chỉ `buttonPressed=true` → `loop()` chớp LED báo ngay + fade nền.
- Project: [bai1b_co_ngat](./bai1b_co_ngat)

## Bài 2: Debounce
- Bản thô `vd1_no_debounce`: ISR `isrCount++`, stream `>pinState/>isrCount` 500Hz → 1 nhấn nhảy nhiều.
- Bản lọc: ISR `millis()` cửa sổ `DEBOUNCE_MS` (thử 5/50/500, chốt 50), stream `>rawEdgeCount/>debouncedCount`.
- Xem đồ thị bằng Teleplot (extension VS Code) đọc `>ten:gia_tri`, không cần script ngoài.
- Project: [bai2_debounce](./bai2_debounce)

## Bài 3a: ADC1 + lọc + LEDC
- Pot GPIO34 → lọc trung bình trượt N=8 → LED GPIO18 `map(0-4095→0-255)`, stream `>tho/>loc` 50Hz.
- Đổi `USE_FILTER 0/1`, `SIM_NOISE 0/1` để so sánh. Xem bằng Teleplot.
- Project: [bai3a_adc1](./bai3a_adc1)

## Bài 3b: ADC2 strapping + Wi-Fi
- Như 3a nhưng pot GPIO15 (ADC2 + strapping) + `WiFi.begin` → đọc sai/hang khi Wi-Fi bật.
- Project: [bai3b_adc2](./bai3b_adc2)

