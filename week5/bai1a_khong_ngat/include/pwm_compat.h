#pragma once
#include <Arduino.h>

// Chay duoc ca Arduino-ESP32 core 2.x va 3.x
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  inline void pwmBegin(uint8_t pin, uint8_t ch, uint32_t freq, uint8_t bits) { ledcAttach(pin, freq, bits); }
  inline void pwmWrite(uint8_t pin, uint8_t ch, uint32_t duty)               { ledcWrite(pin, duty); }
#else
  inline void pwmBegin(uint8_t pin, uint8_t ch, uint32_t freq, uint8_t bits) { ledcSetup(ch, freq, bits); ledcAttachPin(pin, ch); }
  inline void pwmWrite(uint8_t pin, uint8_t ch, uint32_t duty)               { ledcWrite(ch, duty); }
#endif
