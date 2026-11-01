
#include <Arduino.h>

#define LED1 2
#define ADC 34

const uint32_t timerMs = 20;
uint32_t loopTick = 0;

struct ThongKeMillis
{
  uint32_t lastMicros = 0;
  uint32_t count = 0;
  uint32_t dtMin = UINT32_MAX;
  uint32_t dtMax = 0;
  uint64_t dtSum = 0;

  void ghiNhan(uint32_t now)
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
} tkMillis;

void updateLoopTimer()
{
  if (millis() - loopTick < timerMs)
    return;

  loopTick += timerMs;

  if (millis() - loopTick >= timerMs)
    loopTick = millis();

  uint32_t now = micros();
  tkMillis.ghiNhan(now);
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

  Serial.println("--- Demo Ví dụ 1: Timer millis ---");
}

unsigned long printTimer = 0;

void loop()
{
  updateLoopTimer();

  if (millis() - printTimer >= 2000)
  {
    printTimer = millis();

    if (tkMillis.count > 0)
    {
      uint32_t avgUs =
          (uint32_t)(tkMillis.dtSum / tkMillis.count);

      Serial.printf(
          "[Timer millis: %u ms] -> Thuc te: Min=%u us | Max=%u us | Avg=%u us\n",
          timerMs,
          tkMillis.dtMin,
          tkMillis.dtMax,
          avgUs);
    }
  }
}
