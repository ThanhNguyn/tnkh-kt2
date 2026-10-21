# Week 5 (TH1) — Đề bài code

## Bài 1a: LED fading polling
- 2 vòng `for` fade 0→255→0, mỗi bước `pwm + delay(10)`, check nút GPIO4 trong vòng lặp → phản hồi trễ.
- Nút GPIO4 `INPUT_PULLUP`, LED GPIO2 LEDC 5kHz 8-bit.
- Project: [bai1a_khong_ngat](./bai1a_khong_ngat)
- Image: ![alt text](https://github.com/ThanhNguyn/tnkh-kt2/blob/main/week5/bai1a_khong_ngat/bai1a_led_fading_polling_bb.png)


https://github.com/user-attachments/assets/b392c5b5-6644-48fb-93c4-f1555009138f




## Bài 1b: LED fading + ngắt ngoài
- `attachInterrupt(GPIO4, FALLING)` → ISR chỉ `buttonPressed=true` → `loop()` chớp LED báo ngay + fade nền.
- Project: [bai1b_co_ngat](./bai1b_co_ngat)
- Image: ![alt text](https://github.com/ThanhNguyn/tnkh-kt2/blob/main/week5/bai1b_co_ngat/bai1b_led_fading_interrupt_bb.png)


https://github.com/user-attachments/assets/689eb330-e923-408e-860a-f3f0e9671b25



## Bài 2: Debounce
- Bản thô: [bai2_nodebounce](./bai2_nodebounce) — ISR `isrCount++`, stream `>pinState/>isrCount` ~500Hz (`delay(2)`), 1 nhấp nhảy nhiều.
- Bản lọc: [bai2_debounce](./bai2_debounce) — ISR `millis()` cửa sổ `DEBOUNCE_MS` (thử 5/50/500, chốt 50), stream `>pinState/>rawEdgeCount/>debouncedCount` 50Hz.
- Xem đồ thị bằng Teleplot (extension VS Code) đọc `>ten:gia_tri`, không cần script ngoài.
- Image: ![alt text](https://github.com/ThanhNguyn/tnkh-kt2/blob/main/week5/bai2_debounce/bai2_debounce_bb.png)



https://github.com/user-attachments/assets/1b40d74e-6751-49eb-a9d9-743ac230bd30



## Bài 3a: ADC1 + lọc + LEDC
- Pot GPIO34 → lọc trung bình trượt N=8 → LED GPIO18 `map(0-4095→0-255)`, stream `>tho/>loc` 50Hz.
- Đổi `USE_FILTER 0/1`, `SIM_NOISE 0/1` để so sánh. Xem bằng Teleplot.
- Project: [bai3a_adc1](./bai3a_adc1)
- Image: ![alt text](https://github.com/ThanhNguyn/tnkh-kt2/blob/main/week5/bai3a_adc1/bai3a_adc1_filter_ledc_bb.png)


https://github.com/user-attachments/assets/2539e673-7038-4fa4-b9d9-001f0ca25c9a



## Bài 3b: ADC2 strapping + Wi-Fi
- Như 3a nhưng pot GPIO15 (ADC2 + strapping) + `WiFi.begin` → đọc sai/hang khi Wi-Fi bật.
- Project: [bai3b_adc2](./bai3b_adc2)
- Image: ![alt text](https://github.com/ThanhNguyn/tnkh-kt2/blob/main/week5/bai3b_adc2/bai3b_adc2_wifi_bb.png)


