# Week 1 — Đề bài code

## 1. Blink LED
- LED GPIO2 blink `delay(500)`.
- Project: [ESP32_Blink_LED](./ESP32_Blink_LED)

## 2. Blink millis
- LED GPIO2 blink non-blocking `millis()` 500ms + log Serial 115200.
- Project: [ESP32_Blink_Millis](./ESP32_Blink_Millis)

## 3. Button LED
- Nút GPIO4 `INPUT_PULLUP` → LED GPIO2.
- Project: [ESP32_Button_LED](./ESP32_Button_LED)

## 4. ADC Potentiometer
- Biến trở GPIO34, `analogReadResolution(12)` + `analogReadMilliVolts`, log mV mỗi 200ms.
- Project: [ESP32_ADC_Potentiometer](./ESP32_ADC_Potentiometer)

## 5. LED + Button (Serial log)
- LED GPIO5, nút GPIO4 `INPUT_PULLUP`, log `PRESSED`/`RELEASED` 115200, poll 50ms.
- Project: [esp32-led-button](./esp32-led-button)
