/*
 * BAI 2 - VI DU 1: QUAN SAT HIEN TUONG NAY PHIM (BOUNCING)
 * ---------------------------------------------------------------
 * Mach giu nguyen bai 1: nut nhan GPIO4 (INPUT_PULLUP), LED GPIO2.
 *
 * ISR o day KHONG chong doi: moi suon xuong (FALLING) tren chan nut
 * deu lam bien dem tang 1. Do tiep diem co khi nhan/nha (nay ~5-20ms),
 * mot lan nhan that cua ngon tay co the sinh RA NHIEU suon xuong lien
 * tiep -> bien dem se tang nhieu hon 1 cho MOI LAN nhan.
 *
 * Cach quan sat bang do thi (plot) trong VS Code voi extension Teleplot:
 *   1. Cai extension "Teleplot" (alexnesnes.teleplot) trong VS Code.
 *   2. Nap chuong trinh nay vao board (env vd1_no_debounce).
 *   3. Mo Serial Monitor cua VS Code (khong phai cua PlatformIO), tro toi
 *      COM port cua ESP32, baud 115200 -> Teleplot se tu dong nhan dong
 *      du lieu dang ">ten:gia_tri" va ve thanh do thi.
 *   4. Nhan nut 1 LAN DUT KHOAT roi quan sat tren do thi Teleplot:
 *        - pinState: tin hieu 0/1 doc truc tiep tu chan GPIO4 (de thay
 *          dang song nay nhu hinh dao dong ky).
 *        - isrCount: so lan ISR da duoc goi (se nhay vot nhieu don vi
 *          chi trong 1 lan nhan, thay vi dung ra tang 1).
 *   5. So sanh ket qua nay voi vi du 2 (co chong doi).
 */

#include <Arduino.h>

#define BUTTON_PIN 4   // Chan nut nhan (ngat ngoai)
#define LED_PIN    2   // Chan LED bao hieu

volatile uint32_t isrCount = 0;   // Dem so lan ISR duoc goi (KHONG chong doi)

// ----------------- ISR: KHONG CHONG DOI -----------------
void IRAM_ATTR handleInterrupt() {
  isrCount++;   // Moi suon xuong deu duoc dem, ke ca xung nay
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("=== BAI 2 - VD1: Quan sat hien tuong nay phim (KHONG chong doi) ===");
  Serial.println("Nhan nut 1 lan va xem isrCount tang bao nhieu don vi.");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);
}

void loop() {
  // Doc truc tiep trang thai chan de ve dang song tren Serial Plotter
  int pinState = digitalRead(BUTTON_PIN);

  // LED sang theo trang thai nut (chi de quan sat truc quan, khong lien quan debounce)
  digitalWrite(LED_PIN, pinState == LOW ? HIGH : LOW);

  // Dinh dang Teleplot: moi dong ">ten:gia_tri" la 1 duong do thi rieng.
  // pinState: 0 hoac 1 (hien dang song/nay cua nut)
  // isrCount: tong so lan ISR da kich hoat tu luc boot (se "nhay bac thang")
  Serial.print(">pinState:");
  Serial.println(pinState);
  Serial.print(">isrCount:");
  Serial.println(isrCount);

  delay(2);   // Lay mau nhanh (~500Hz) de khong bo sot xung nay ngan 5-20ms
}
