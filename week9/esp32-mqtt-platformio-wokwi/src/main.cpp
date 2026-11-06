
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#if defined(WOKWI_SIMULATION)
// Wokwi simulation: DHT22 + BMP180
#include <DHTesp.h>
#include <Adafruit_BMP085.h>
#else
// Real hardware: AHT20 + BMP280
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#endif

// ============================================================
// 1. WIFI CONFIGURATION
// ============================================================

#if defined(WOKWI_SIMULATION)

const char *ssid = "Wokwi-GUEST";
const char *password = "";

#else

const char *ssid = "WIFI_SSID";
const char *password = "WIFI_PASSWORD";

#endif

// ============================================================
// 2. MQTT CONFIGURATION
// ============================================================

// MQTT Broker
const char *mqtt_server = "broker.hivemq.com";
const uint16_t mqtt_port = 1883;

// MQTT Topics
const char *topic_sensor = "esp32/sensors";
const char *topic_led = "esp32/led";

// MQTT Client ID
String mqttClientId;

// ============================================================
// 3. GPIO CONFIGURATION
// ============================================================

#define LED_PIN 18
#define BUTTON_PIN 4

#define SDA_PIN 21
#define SCL_PIN 22

#if defined(WOKWI_SIMULATION)
#define DHT_PIN 16
#endif

// ============================================================
// 4. TIMING CONFIGURATION
// ============================================================

const unsigned long publishInterval = 5000;
const unsigned long wifiRetryInterval = 10000;
const unsigned long mqttRetryInterval = 5000;

unsigned long lastPublish = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;

// ============================================================
// 5. MQTT AND SENSOR OBJECTS
// ============================================================

WiFiClient espClient;
PubSubClient client(espClient);

#if defined(WOKWI_SIMULATION)

DHTesp dht;
Adafruit_BMP085 bmp180;

bool pressureSensorReady = false;

#else

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp280;

bool ahtReady = false;
bool pressureSensorReady = false;

#endif

bool wifiStarted = false;
bool wasWiFiConnected = false;

// ============================================================
// 6. SETUP WIFI
// ============================================================

void setup_wifi()
{

  Serial.println();
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

#if defined(WOKWI_SIMULATION)

  WiFi.begin(ssid, password, 6);

#else

  WiFi.begin(ssid, password);

#endif

  wifiStarted = true;
  lastWiFiAttempt = millis();
}

// ============================================================
// 7. MQTT CALLBACK
// Receive commands from MQTT Broker
// ============================================================

void callback(char *topic, byte *message, unsigned int length)
{

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  if (strcmp(topic, topic_led) != 0)
  {
    Serial.println("Ignored topic");
    return;
  }

  const size_t bufferSize = 32;

  if (length == 0 || length >= bufferSize)
  {
    Serial.println("Invalid command length");
    return;
  }

  char command[bufferSize] = {0};

  memcpy(command, message, length);
  command[length] = '\0';

  // Remove whitespace at the beginning
  size_t start = 0;

  while (
      command[start] &&
      isspace(static_cast<unsigned char>(command[start])))
  {
    start++;
  }

  // Remove whitespace at the end
  size_t end = strlen(command);

  while (
      end > start &&
      isspace(static_cast<unsigned char>(command[end - 1])))
  {
    command[--end] = '\0';
  }

  // Convert to uppercase
  for (size_t i = start; i < end; i++)
  {

    command[i] = static_cast<char>(
        toupper(static_cast<unsigned char>(command[i])));
  }

  const char *value = command + start;

  Serial.println(value);

  // Turn LED ON
  if (
      strcmp(value, "1") == 0 ||
      strcmp(value, "ON") == 0)
  {

    digitalWrite(LED_PIN, HIGH);

    Serial.println("LED -> ON");
  }

  // Turn LED OFF
  else if (
      strcmp(value, "0") == 0 ||
      strcmp(value, "OFF") == 0)
  {

    digitalWrite(LED_PIN, LOW);

    Serial.println("LED -> OFF");
  }

  else
  {

    Serial.println("Invalid command. Use 1/ON or 0/OFF.");
  }
}

// ============================================================
// 8. RECONNECT MQTT
// ============================================================

void reconnect()
{

  if (client.connected())
  {
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    return;
  }

  unsigned long now = millis();

  if (now - lastMQTTAttempt < mqttRetryInterval)
  {
    return;
  }

  lastMQTTAttempt = now;

  Serial.print("Attempting MQTT connection to ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.print(mqtt_port);
  Serial.print(" ... ");

  // Connect to MQTT Broker
  if (client.connect(mqttClientId.c_str()))
  {

    Serial.println("connected!");

    // Subscribe to LED control topic
    if (client.subscribe(topic_led))
    {

      Serial.print("Subscribed to: ");
      Serial.println(topic_led);
    }

    else
    {

      Serial.println("Subscribe failed!");
    }
  }

  else
  {

    Serial.print("failed, rc=");
    Serial.println(client.state());
  }
}

// ============================================================
// 9. INITIALIZE SENSORS
// ============================================================

void setup_sensors()
{

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

#if defined(WOKWI_SIMULATION)

  // DHT22
  dht.setup(DHT_PIN, DHTesp::DHT22);

  Serial.println("DHT22 initialized!");

  // BMP180
  pressureSensorReady = bmp180.begin();

  if (pressureSensorReady)
  {

    Serial.println("BMP180 initialized!");
  }

  else
  {

    Serial.println("WARNING: BMP180 not detected!");
  }

#else

  // AHT20
  ahtReady = aht.begin(&Wire);

  if (ahtReady)
  {

    Serial.println("AHT20 initialized!");
  }

  else
  {

    Serial.println("ERROR: AHT20 not detected!");
  }

  // BMP280
  pressureSensorReady = bmp280.begin(0x76);

  if (!pressureSensorReady)
  {
    pressureSensorReady = bmp280.begin(0x77);
  }

  if (pressureSensorReady)
  {

    Serial.println("BMP280 initialized!");
  }

  else
  {

    Serial.println("WARNING: BMP280 not detected!");
  }

#endif
}

// ============================================================
// 10. READ SENSOR DATA
// ============================================================

bool read_sensor_data(
    float &temperature,
    float &humidity,
    float &pressure,
    bool &pressureValid)
{

  pressureValid = false;

#if defined(WOKWI_SIMULATION)

  // Read DHT22
  TempAndHumidity data = dht.getTempAndHumidity();

  temperature = data.temperature;
  humidity = data.humidity;

  if (
      !isfinite(temperature) ||
      !isfinite(humidity))
  {

    Serial.println("DHT22 read failed!");
    return false;
  }

  // Read BMP180
  if (pressureSensorReady)
  {

    // Pascal -> hPa
    pressure = bmp180.readPressure() / 100.0F;

    pressureValid =
        isfinite(pressure) &&
        pressure > 0;
  }

#else

  if (!ahtReady)
  {

    Serial.println("AHT20 is unavailable!");
    return false;
  }

  // Read AHT20
  sensors_event_t humidityEvent;
  sensors_event_t temperatureEvent;

  aht.getEvent(&humidityEvent, &temperatureEvent);

  temperature = temperatureEvent.temperature;
  humidity = humidityEvent.relative_humidity;

  if (
      !isfinite(temperature) ||
      !isfinite(humidity))
  {

    Serial.println("AHT20 read failed!");
    return false;
  }

  // Read BMP280
  if (pressureSensorReady)
  {

    // Pascal -> hPa
    pressure = bmp280.readPressure() / 100.0F;

    pressureValid =
        isfinite(pressure) &&
        pressure > 0;
  }

#endif

  return true;
}

// ============================================================
// 11. PUBLISH SENSOR DATA
// Publish JSON every 5 seconds
// ============================================================

void publish_sensor_data()
{

  float temperature = NAN;
  float humidity = NAN;
  float pressure = NAN;

  bool pressureValid = false;

  // Read sensor data
  if (
      !read_sensor_data(
          temperature,
          humidity,
          pressure,
          pressureValid))
  {

    Serial.println("Sensor read failed!");
    return;
  }

  // INPUT_PULLUP:
  // LOW = button pressed
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // JSON payload
  char payload[192];

  if (pressureValid)
  {

    snprintf(
        payload,
        sizeof(payload),

        "{\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
        "\"button\":%s}",

        temperature,
        humidity,
        pressure,
        buttonPressed ? "true" : "false");
  }

  else
  {

    snprintf(
        payload,
        sizeof(payload),

        "{\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":null,"
        "\"button\":%s}",

        temperature,
        humidity,
        buttonPressed ? "true" : "false");
  }

  Serial.println();
  Serial.println("========== MQTT PUBLISH ==========");

  Serial.print("Topic: ");
  Serial.println(topic_sensor);

  Serial.print("Payload: ");
  Serial.println(payload);

  // Publish JSON
  if (client.publish(topic_sensor, payload))
  {

    Serial.println("Publish successful!");
  }

  else
  {

    Serial.println("MQTT Publish failed!");
  }

  Serial.println("==================================");
}

// ============================================================
// 12. SETUP
// ============================================================

void setup()
{

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("====================================");
  Serial.println(" ESP32 MQTT PUBLISH / SUBSCRIBE");
  Serial.println("====================================");

#if defined(WOKWI_SIMULATION)

  Serial.println("Mode: Wokwi Simulation");

#else

  Serial.println("Mode: Real Hardware");

#endif

  // Configure GPIO
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);

  // Initialize sensors
  setup_sensors();

  // MQTT configuration
  client.setServer(mqtt_server, mqtt_port);

  client.setCallback(callback);

  client.setBufferSize(512);
  client.setKeepAlive(30);
  client.setSocketTimeout(3);

  // Unique MQTT Client ID
  mqttClientId = "ESP32-";

  String mac = WiFi.macAddress();
  mac.replace(":", "");

  mqttClientId += mac;

  Serial.print("MQTT Client ID: ");
  Serial.println(mqttClientId);

  Serial.print("MQTT Broker: ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port);

  // Connect Wi-Fi
  setup_wifi();
}

// ============================================================
// 13. MAIN LOOP
// ============================================================

void loop()
{

  unsigned long now = millis();

  // --------------------------------------------------------
  // Wi-Fi connection monitoring
  // --------------------------------------------------------

  if (WiFi.status() != WL_CONNECTED)
  {

    if (wasWiFiConnected)
    {

      Serial.println("Wi-Fi disconnected!");

      client.disconnect();

      wasWiFiConnected = false;
    }

    // Retry Wi-Fi
    if (
        !wifiStarted ||
        now - lastWiFiAttempt >= wifiRetryInterval)
    {

      Serial.println("Reconnecting Wi-Fi...");

      setup_wifi();
    }
  }

  else
  {

    if (!wasWiFiConnected)
    {

      Serial.println();
      Serial.println("Wi-Fi connected!");

      Serial.print("ESP32 IP: ");
      Serial.println(WiFi.localIP());

      wasWiFiConnected = true;
    }

    // ----------------------------------------------------
    // MQTT reconnect
    // ----------------------------------------------------

    if (!client.connected())
    {

      reconnect();
    }

    // Process incoming MQTT messages
    if (client.connected())
    {

      client.loop();
    }
  }

  // --------------------------------------------------------
  // Periodic sensor publishing
  // --------------------------------------------------------

  now = millis();

  if (
      client.connected() &&
      now - lastPublish >= publishInterval)
  {

    lastPublish = now;

    publish_sensor_data();
  }

  delay(2);
}
