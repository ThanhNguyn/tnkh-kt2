# Week 2 — Ngoại vi ESP32 (L02)

Nguồn đề: `L02-buoi2-v2.pptx` slide 2-3, 48-50. BTVN không nộp nhưng Lab 1 tuần 3 bắt đầu từ đây, lưu vào `buoi2/`.

## Đề chuẩn (5 việc)

1. Mở 3 mạch Wokwi (LED sáng dần + nút bản 1–3, Demo 1, Demo 2), chạy + thử đổi 1 tham số: debounce / chu kỳ timer / tần số LEDC → link trong `buoi2/README.md`.
2. Ghép Demo 1 + Demo 2: nút đếm bằng ngắt, LED1 sáng theo số lần nhấn (LEDC), LED2 nháy 250ms bằng timer, `loop()` không `delay()` → `buoi2/ghep/`.
3. Bảng chân dự kiến đồ án (≥5 ngoại vi), mỗi chân 1 lý do → `buoi2/bang-chan.md`.
4. Đọc datasheet 1 module, trả lời 4 câu hỏi → Lab Notebook.
5. Cài VS Code + PlatformIO, build `esp32dev` SUCCESS → ảnh SUCCESS.

## Kiến thức trọng tâm (slide 48)

- Khác Uno: ESP32 3.3V, ~12mA/chân, GPIO matrix tự chọn chân.
- Ngắt ngoài: ISR chỉ đặt cờ, `IRAM_ATTR`, biến chung `volatile`, nút cần debounce 50ms.
- Chân cấm: không dùng GPIO6–11 (flash); strapping 0,2,5,12,15 không kéo mức lúc boot; 34–39 input-only, không pull nội.
- Timer HW: tự đếm + gọi ISR đúng chu kỳ, ISR đặt cờ, `loop()` xử lý.
- LEDC: LED 5kHz 8-bit, servo 50Hz 16-bit.
- ADC: số thô lệch, dùng `analogReadMilliVolts` + trung bình, cảm biến analog vào ADC1 (32–39) vì ADC2 mất khi Wi-Fi bật.
- Bus: 3 UART, 2 I2C, 2 SPI, chân tùy chọn.

## Map project trong folder này

- [demo1](./demo1) — ngắt ngoài GPIO4 FALLING + debounce 50ms (`millis()` trong ISR + `portMUX`), LED GPIO2.
- [demo2](./demo2) — LEDC GPIO2 5kHz 8-bit + timer HW 100ms + ADC GPIO34 → PWM.
- [demo3](./demo3) — so sánh ADC raw / lý thuyết (`raw*3300/4095`) / hiệu chuẩn (`analogReadMilliVolts`), trung bình 16 mẫu. Xem [WIRING](./demo3/WIRING.md).
- [demo4](./demo4) — demo GPIO12 strapping (MTDI, flash voltage).
- [ghep-demo1-demo2](./ghep-demo1-demo2) — đúng spec việc 2: nút ngắt → LED1 PWM 5 mức, LED2 timer 250ms, không `delay()`.
- [led-sang-dan](./led-sang-dan) — biến thể nút đổi 5 mức sáng (đang dùng `delay(50)` debounce, nên đổi sang `millis()`).
- [ESP32_Lab1_Bac4_OLED_DHT11](./ESP32_Lab1_Bac4_OLED_DHT11) — Lab1 bậc 4: nút GPIO18 ngắt + LED GPIO4 LEDC + DHT GPIO27 2s + OLED I2C 21/22 0x3C.
- [ESP32_PlatformIO_Wokwi_Setup](./ESP32_PlatformIO_Wokwi_Setup) — template blink `millis()` 500ms.

## Minh chứng

- [ ] Link 3 mạch Wokwi + tham số đã đổi
- [ ] Folder `ghep/` 1 commit
- [ ] `bang-chan.md` ≥5 ngoại vi
- [ ] Notebook datasheet 4 câu hỏi
- [ ] Ảnh build SUCCESS

## Ôn tập (slide 49)

1. ISR có `Serial.println` vi phạm gì? Sửa sao vẫn in được số lần nhấn?
2. Đề xuất bảng chân 3 nút + relay + ẩm đất analog + OLED I2C + Wi-Fi.
3. Vì sao `raw*3300/4095` lệch trên ESP32 mà `raw*5000/1023` gần đúng trên Uno?
4. Servo 0.5–2.5ms 50Hz, LEDC 16-bit thì duty 0°/180° bao nhiêu?
5. Khi nào `millis()` không đủ, phải timer HW?
6. OLED ghi VCC 3.3–5V I2C 0x3C: 4 câu hỏi datasheet khi nối ESP32?
