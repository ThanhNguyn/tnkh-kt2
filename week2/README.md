# Week 2 — Đề bài code

## Demo 1: ngắt ngoài + debounce
- Nút GPIO4 `INPUT_PULLUP`, ngắt `FALLING`, debounce 50ms bằng `millis()` trong ISR.
- LED GPIO2 đổi trạng thái mỗi lần nhấn, Serial số lần nhấn 115200.
- Project: [demo1](./demo1)

## Demo 2: LEDC + timer + ADC
- LED GPIO2 LEDC 5kHz 8-bit, timer HW 100ms đọc ADC GPIO34 → `duty = raw/16`.
- Project: [demo2](./demo2)

## Demo 3: ADC raw / lý thuyết / hiệu chuẩn
- Pot GPIO34, trung bình 16 mẫu, so sánh `raw` vs `raw*3300/4095` vs `analogReadMilliVolts`.
- Project: [demo3](./demo3)

## Demo 4: GPIO12 strapping
- Đọc mức GPIO12 sau boot + blink, demo kéo HIGH lúc boot gây `flash read err`.
- Project: [demo4](./demo4)

## Ghép Demo 1 + 2
- Nút ngắt → LED1 GPIO2 PWM 5 mức (0,64,128,192,255), LED2 GPIO5 timer 250ms, `loop()` không `delay()`.
- Project: [ghep-demo1-demo2](./ghep-demo1-demo2)

## Lab 1 bậc 4: OLED + DHT
- Nút GPIO18 ngắt + debounce 50ms → LED GPIO4 LEDC.
- DHT GPIO27 đọc mỗi 2s + OLED I2C SDA21/SCL22 0x3C hiện T/H.
- Project: [ESP32_Lab1_Bac4_OLED_DHT11](./ESP32_Lab1_Bac4_OLED_DHT11)
