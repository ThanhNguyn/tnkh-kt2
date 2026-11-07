#include <Arduino.h>
#include <NimBLEDevice.h>

namespace
{
    constexpr char kDeviceName[] = "TH3-BLE";
    constexpr char kServiceUuid[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    constexpr char kCharacteristicUuid[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
    constexpr char kInitialValue[] = "TH3 BLE Server Ready";

    class ServerCallbacks final : public NimBLEServerCallbacks
    {
    public:
        void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override
        {
            (void)server;
            Serial.println("[BLE] Client connected");
        }

        void onDisconnect(NimBLEServer *server,
                          NimBLEConnInfo &connInfo,
                          int reason) override
        {
            (void)server;
            (void)connInfo;
            Serial.printf("[BLE] Client disconnected (reason=%d)\n", reason);
            NimBLEDevice::startAdvertising();
            Serial.println("[BLE] Advertising restarted");
        }
    };

    class CharacteristicCallbacks final : public NimBLECharacteristicCallbacks
    {
    public:
        void onRead(NimBLECharacteristic *characteristic,
                    NimBLEConnInfo &connInfo) override
        {
            (void)connInfo;

            const std::string value = characteristic->getValue();
            Serial.printf("[BLE] READ (%u bytes)\n",
                          static_cast<unsigned>(value.size()));
        }

        void onWrite(NimBLECharacteristic *characteristic,
                     NimBLEConnInfo &connInfo) override
        {
            (void)connInfo;

            const std::string value = characteristic->getValue();
            Serial.printf("[BLE] WRITE (%u bytes): ",
                          static_cast<unsigned>(value.size()));
            Serial.write(reinterpret_cast<const uint8_t *>(value.data()), value.size());
            Serial.println();
        }
    };

    ServerCallbacks g_serverCallbacks;
    CharacteristicCallbacks g_characteristicCallbacks;
} // namespace

void setup()
{
    Serial.begin(115200);
    delay(300);

    Serial.println();
    Serial.println("========================================");
    Serial.println("TH3 - BAI 2 - BLE GATT SERVER");
    Serial.println("========================================");

    NimBLEDevice::init(kDeviceName);
    Serial.println("[BLE] Waiting for host/controller to settle...");
    delay(1000);
    Serial.println("[BLE] Host/controller settle delay completed.");
    NimBLEServer *server = NimBLEDevice::createServer();
    if (server == nullptr)
    {
        Serial.println("[ERROR] Failed to create BLE server");
        return;
    }
    server->setCallbacks(&g_serverCallbacks);

    NimBLEService *service = server->createService(kServiceUuid);
    if (service == nullptr)
    {
        Serial.println("[ERROR] Failed to create BLE service");
        return;
    }

    NimBLECharacteristic *characteristic = service->createCharacteristic(
        kCharacteristicUuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    if (characteristic == nullptr)
    {
        Serial.println("[ERROR] Failed to create BLE characteristic");
        return;
    }

    characteristic->setCallbacks(&g_characteristicCallbacks);
    characteristic->setValue(kInitialValue);

    if (!server->start())
    {
        Serial.println("[ERROR] GATT server start failed");
        return;
    }

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(kServiceUuid);
    advertising->setName(kDeviceName);

    const bool advertisingStarted = advertising->start();

    Serial.printf(
        "[BLE] advertising->start() returned: %s\n",
        advertisingStarted ? "true" : "false");

    if (!advertisingStarted)
    {
        Serial.println("[ERROR] BLE advertising start failed");
        return;
    }

    Serial.println("[OK] BLE GATT server started");
    Serial.printf("Device name: %s\n", kDeviceName);
    Serial.printf("Service UUID: %s\n", kServiceUuid);
    Serial.printf("Characteristic UUID: %s\n", kCharacteristicUuid);
    Serial.printf("Initial value: %s\n", kInitialValue);
    Serial.println("Properties: READ | WRITE");
    Serial.println("[OK] BLE advertising started");
}

void loop()
{
    delay(1000);
}
