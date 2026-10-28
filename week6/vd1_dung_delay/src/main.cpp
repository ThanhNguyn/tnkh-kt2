#include <Arduino.h>

#define LED1 2
#define ADC 34
// put function declarations here:
void setup_vd1();
void loop_vd1();
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  pinMode(LED1, OUTPUT);
  pinMode(ADC, INPUT);

  Serial.println("---System initialized---");
}

void loop()
{
  digitalWrite(LED1, HIGH);
  delay(500);
  digitalWrite(LED1, LOW);
  delay(500);

  int tho = analogRead(ADC);
  Serial.println(tho);
  delay(2000);

  Serial.println("heartBeat");
  delay(5000);
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}