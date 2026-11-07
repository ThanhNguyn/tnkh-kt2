# TH3 - Bai 2: BLE GATT Server

PlatformIO firmware for ESP32-S3 using NimBLE-Arduino.

- BLE device name: `TH3-BLE`
- GATT service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- GATT characteristic UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Initial characteristic value: `TH3 BLE Server Ready`
- Characteristic properties: READ and WRITE

The `tools` directory contains a host-side Bumble GATT test client for software-only ESP-EMU testing.
