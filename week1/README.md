# Week 1 — GPIO cơ bản (L01)

Nguồn đề: `L01-buoi1-full-v2.pptx` slide 55 + `On-tap-nhanh-Arduino.pptx` (7 mảng TNKHKT1, ôn, không điểm).

## Đề chuẩn (BTVN trước tuần 2)

1. Làm lại 4 demo `Blink · Nút nhấn · millis · ADC` cả Wokwi + kit thật qua PlatformIO → nộp link Wokwi + ảnh Serial Monitor.
2. TK GitHub + repo `tnkhkt2-<mssv>` commit 4 ví dụ → link repo.
3. Nửa trang “Chọn nền tảng cho bài toán X”, so sánh ≥3 nền tảng → file PDF.
4. Phiếu ôn nhanh mảng dưới ngưỡng → trước Lab 1.

## Map project trong folder này

- [ESP32_Blink_LED](./ESP32_Blink_LED) — Blink `delay(500)` LED GPIO2.
- [ESP32_Blink_Millis](./ESP32_Blink_Millis) — Blink non-blocking `millis()` 500ms + log Serial 115200.
- [ESP32_Button_LED](./ESP32_Button_LED) — nút GPIO4 `INPUT_PULLUP` → LED GPIO2.
- [ESP32_ADC_Potentiometer](./ESP32_ADC_Potentiometer) — pot GPIO34, `analogReadResolution(12)` + `analogReadMilliVolts`, log mV mỗi 200ms.
- [esp32-led-button](./esp32-led-button) — biến thể button GPIO4 + LED GPIO5 + Serial PRESSED/RELEASED (chú ý GPIO5 là strapping, demo nên chuyển về GPIO2).

## Minh chứng cần nộp

- [ ] Link Wokwi 4 demo
- [ ] Ảnh Serial Monitor
- [ ] Link repo `tnkhkt2-<mssv>`
- [ ] PDF so sánh nền tảng
