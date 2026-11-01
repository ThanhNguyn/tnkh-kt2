
#include <Arduino.h>

#define LED1 2
#define ADC 34

const uint32_t timerMs = 20;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

struct ThongKeHardwareTimer
{
  uint32_t lastMicros = 0;
  uint32_t count = 0;
  uint32_t dtMin = UINT32_MAX;
  uint32_t dtMax = 0;
  uint64_t dtSum = 0;

  void IRAM_ATTR ghiNhan(uint32_t now)
  {
    if (lastMicros != 0)
    {
      uint32_t dt = now - lastMicros;

      if (dt < dtMin)
        dtMin = dt;

      if (dt > dtMax)
        dtMax = dt;

      dtSum += dt;
      count++;
    }

    lastMicros = now;
  }
} tkHw;

hw_timer_t *timer = nullptr;

void IRAM_ATTR onTimer()
{
  uint32_t now = micros();

  portENTER_CRITICAL_ISR(&timerMux);
  tkHw.ghiNhan(now);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void khoiTaoTimer()
{
  timer = timerBegin(1000000);

  if (timer == nullptr)
  {
    Serial.println("Loi: Khong the khoi tao Hardware Timer!");
    return;
  }

  timerAttachInterrupt(timer, &onTimer);

  timerAlarm(timer, (uint64_t)timerMs * 1000ULL, true, 0);

  Serial.println("Hardware Timer da khoi tao thanh cong!");
}

void setup()
{
  Serial.begin(115200);

  while (!Serial)
  {
    delay(10);
  }

  pinMode(LED1, OUTPUT);
  pinMode(ADC, INPUT);

  Serial.println("--- Demo Vi du 2: Hardware Timer ---");

  khoiTaoTimer();
}

uint32_t printTimer = 0;

void loop()
{
  if (millis() - printTimer >= 2000)
  {
    printTimer = millis();

    uint32_t count;
    uint32_t dtMin;
    uint32_t dtMax;
    uint64_t dtSum;

    portENTER_CRITICAL(&timerMux);

    count = tkHw.count;
    dtMin = tkHw.dtMin;
    dtMax = tkHw.dtMax;
    dtSum = tkHw.dtSum;

    portEXIT_CRITICAL(&timerMux);

    if (count > 0)
    {
      uint32_t avgUs = (uint32_t)(dtSum / count);

      Serial.printf(
          "[Timer HW: %u ms] -> Thuc te: Min=%u us | Max=%u us | Avg=%u us\n",
          timerMs,
          dtMin,
          dtMax,
          avgUs);
    }
  }
}
