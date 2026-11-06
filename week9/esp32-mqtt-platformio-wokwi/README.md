# ESP32 MQTT Publish/Subscribe — PlatformIO + Wokwi

This project ports the programming pattern from Random Nerd Tutorials' ESP32 MQTT Publish/Subscribe tutorial to PlatformIO. It keeps the tutorial's recognizable flow:

- `setup_wifi()` starts the Wi-Fi connection.
- `callback()` handles incoming MQTT commands.
- `reconnect()` reconnects to the broker and subscribes again.
- `client.loop()` processes incoming MQTT packets.
- `publish_sensor_data()` sends sensor JSON every five seconds.

Reference: https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/

## 1. MQTT endpoint used in this project

The endpoint you supplied is configured as the MQTT broker:

- Broker IP: `172.16.6.164`
- Broker port: `2100`
- Publish topic: `esp32/sensors`
- Subscribe topic: `esp32/led`
- Publish interval: 5 seconds

If `172.16.6.164:2100` is the PC running Wokwi Gateway rather than the MQTT broker itself, change `mqtt_server` and `MQTT_PORT` in `src/main.cpp` to the actual broker's listening IP and port. A gateway is a network bridge; it is not automatically an MQTT broker.

The code currently uses an unauthenticated MQTT connection because no MQTT username/password was provided. If the broker requires authentication, change:

```cpp
client.connect(mqttClientId.c_str())
```

to:

```cpp
client.connect(mqttClientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)
```

and define those credentials locally.

## 2. Sensor mapping

The assignment specifies AHT20 and three readings: temperature, humidity, pressure. AHT20 measures temperature and humidity only, so the real-hardware build adds a BMP280 for pressure.

- Real hardware (`esp32dev`): AHT20 + BMP280.
- Wokwi (`esp32dev_wokwi`): DHT22 + BMP180 substitutes because those parts are available in the Wokwi simulator.

The published JSON schema is identical in both environments:

```json
{"temperature":24.00,"humidity":55.00,"pressure":1013.25,"button":false}
```

Pressure is in hPa. If the pressure sensor is not available, the JSON field is `null`, never a fabricated value.

## 3. Pin mapping — physical ESP32

| Device | Device pin | ESP32 |
|---|---|---|
| AHT20 | VIN | 3V3 |
| AHT20 | GND | GND |
| AHT20 | SDA | GPIO21 |
| AHT20 | SCL | GPIO22 |
| BMP280 | VIN | 3V3 (only if supported by the breakout) |
| BMP280 | GND | GND |
| BMP280 | SDA | GPIO21 |
| BMP280 | SCL | GPIO22 |
| LED | Anode via 220–330 Ω resistor | GPIO18 |
| LED | Cathode | GND |
| Button | Terminal 1 | GPIO4 |
| Button | Terminal 2 | GND |

Both I2C sensors share GPIO21/22. The button uses `INPUT_PULLUP`; pressed reads `LOW` and is published as JSON `true`.

## 4. PlatformIO environments

`platformio.ini` contains two environments:

- `esp32dev`: physical ESP32 with AHT20 + BMP280 and the `VJU office` Wi-Fi credentials.
- `esp32dev_wokwi`: Wokwi ESP32 with DHT22 + BMP180 and `Wokwi-GUEST` Wi-Fi.

Open this folder in VS Code with PlatformIO IDE and Wokwi extensions installed.

### Build and upload to the physical ESP32

```powershell
pio run -e esp32dev
pio run -e esp32dev -t upload
pio device monitor -b 115200
```

### Build and start Wokwi

```powershell
pio run -e esp32dev_wokwi
```

Then press `Ctrl+Shift+P` in VS Code and run `Wokwi: Start Simulator`. `wokwi.toml` points to the Wokwi-specific firmware build. Wokwi does not build the PlatformIO firmware itself; run the build command first.

## 5. Wokwi to the private MQTT broker

The simulated ESP32 joins `Wokwi-GUEST`. The default public Wokwi gateway provides internet access but cannot reach private LAN addresses such as `172.16.6.164`.

To test against the supplied LAN broker, enable the **Private Wokwi IoT Gateway** in the Wokwi VS Code extension (the gateway feature requires an eligible paid Wokwi plan). The PC running the gateway must have network access to `172.16.6.164:2100`.

If you only want to test the circuit and serial output, Wokwi can run without connecting to the real broker; the MQTT client will retry periodically.

## 6. MQTT test commands

Run these from a machine that can reach the broker and has Mosquitto clients installed:

```powershell
mosquitto_sub -h 172.16.6.164 -p 2100 -t esp32/sensors -v
```

Turn the LED on:

```powershell
mosquitto_pub -h 172.16.6.164 -p 2100 -t esp32/led -m ON
```

Turn the LED off:

```powershell
mosquitto_pub -h 172.16.6.164 -p 2100 -t esp32/led -m OFF
```

If the broker requires authentication, add the corresponding `-u` and `-P` options.

## 7. Important network checks

From the PC, verify that the broker port is reachable:

```powershell
Test-NetConnection 172.16.6.164 -Port 2100
```

A successful result confirms TCP reachability from that PC, but the broker must also be configured to listen on that IP/port and accept the MQTT client. For a physical ESP32, the `VJU office` Wi-Fi network must be able to route to that broker address.

## 8. Libraries

PlatformIO installs these dependencies automatically from `platformio.ini`:

- PubSubClient
- Adafruit AHTX0
- Adafruit BMP280 Library
- Adafruit BMP085 Library (Wokwi BMP180)
- DHT sensor library for ESPx / `DHTesp` (Wokwi DHT22; recommended in Wokwi's ESP32 DHT22 documentation)
- Adafruit Unified Sensor
- Adafruit BusIO

## Security

The Wi-Fi credentials were supplied in the conversation and are included for this lab build. Keep the project private; remove them or move them to an ignored local configuration file before publishing the repository publicly.
