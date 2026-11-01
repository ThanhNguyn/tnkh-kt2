#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const char *deviceName = "ESP32-BLE-NHOMXX";
const char *initialValue = "TH3-ESP32";

class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *characteristic) override
  {
    String value = characteristic->getValue();
    Serial.print("Received value: ");
    Serial.println(value);
  }
};

void setup()
{
  Serial.begin(115200);

  BLEDevice::init(deviceName);

  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(SERVICE_UUID);

  BLECharacteristic *characteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  characteristic->setCallbacks(new CharacteristicCallbacks());
  characteristic->setValue(initialValue);

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();

  Serial.println("BLE server started");
  Serial.print("Device name: ");
  Serial.println(deviceName);
  Serial.print("Service UUID: ");
  Serial.println(SERVICE_UUID);
  Serial.print("Characteristic UUID: ");
  Serial.println(CHARACTERISTIC_UUID);
  Serial.print("Initial value: ");
  Serial.println(initialValue);
}

void loop()
{
  delay(1000);
}
