#include <Arduino.h>

#define LED1 2
#define ADC 34

unsigned long lanTruocLed = 0;
unsigned long lanTruocAdc = 0;
unsigned long lanTruocHeart = 0;

bool ledState = LOW;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }

  pinMode(LED1, OUTPUT);
  pinMode(ADC, INPUT);
}

void loop()
{
  unsigned long now = millis();

  if (now - lanTruocLed >= 500)
  {
    lanTruocLed = now;
    ledState = !ledState;
    digitalWrite(LED1, ledState);
  }

  if (now - lanTruocAdc >= 2000)
  {
    lanTruocAdc = now;
    int tho = analogRead(ADC);
    Serial.println(tho);
  }

  if (now - lanTruocHeart >= 5000)
  {
    lanTruocHeart = now;
    Serial.println("heartbeat");
  }
}