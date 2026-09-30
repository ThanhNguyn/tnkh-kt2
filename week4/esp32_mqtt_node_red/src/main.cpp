#include <Arduino.h>
#include <cmath>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>

// ---------- Chân ----------
const uint8_t DHT_PIN = 4;  // DATA của DHT22
const uint8_t LED_PIN = 2;  // LED (qua điện trở 220 ohm)
#define DHT_TYPE DHT22

// ---------- Wi-Fi (mạng ảo của Wokwi) ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int WIFI_CHANNEL = 6;  // Wokwi phát ở kênh 6, chỉ định sẵn để kết nối nhanh hơn

// ---------- MQTT ----------
const char* MQTT_HOST = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;
const char* TOPIC_SENSOR = "tnkh/week4/esp32/sensor";
const char* TOPIC_LED = "tnkh/week4/esp32/led";
const char* TOPIC_API = "tnkh/week4/esp32/api/sensor";

const uint32_t SENSOR_INTERVAL_MS = 2000;  // đọc DHT22 + publish mỗi 2 giây
const uint32_t MQTT_RETRY_MS = 5000;       // thử nối lại MQTT mỗi 5 giây

// ---------- Đối tượng ----------
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// ---------- Trạng thái ----------
float lastTemperature = NAN;
float lastHumidity = NAN;
bool sensorValid = false;  // true khi lần đọc DHT22 gần nhất thành công
uint32_t lastSensorMs = 0;
uint32_t lastMqttTryMs = 0;
bool mqttTried = false;
char clientId[32];

// ============================================================
// JSON
// ============================================================

// Body của GET /api/sensor (cũng là chuỗi gửi lên TOPIC_API)
void buildApiJson(char* out, size_t size) {
  snprintf(out, size,
           "{\"temperature\":%.1f,\"humidity\":%.1f,"
           "\"unit_temperature\":\"C\",\"unit_humidity\":\"%%\","
           "\"mqtt_topic\":\"%s\"}",
           lastTemperature, lastHumidity, TOPIC_SENSOR);
}

// Payload MQTT theo đề bài (TOPIC_SENSOR)
void buildSensorJson(char* out, size_t size) {
  snprintf(out, size, "{\"temperature\":%.1f,\"humidity\":%.1f}",
           lastTemperature, lastHumidity);
}

// ============================================================
// HTTP
// ============================================================

void handleApiSensor() {
  const String ip = server.client().remoteIP().toString();
  server.sendHeader("Cache-Control", "no-store");

  // DHT22 chưa đọc được thì báo lỗi 503, không trả số 0 giả
  if (!sensorValid) {
    Serial.printf("[HTTP] GET /api/sensor from %s -> 503 (DHT22 has no valid data yet)\n", ip.c_str());
    server.send(503, "application/json",
                "{\"error\":\"sensor_not_ready\","
                "\"message\":\"DHT22 has not returned a valid reading yet\"}");
    return;
  }

  char body[192];
  buildApiJson(body, sizeof(body));
  Serial.printf("[HTTP] GET /api/sensor from %s -> 200 %s\n", ip.c_str(), body);
  server.send(200, "application/json", body);
}

void handleNotFound() {
  server.send(404, "application/json",
              "{\"error\":\"not_found\",\"message\":\"Use GET /api/sensor\"}");
}

// ============================================================
// LED + MQTT nhận lệnh
// ============================================================

void setLed(bool on) {
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  Serial.printf("[LED] %s (GPIO%u = %s)\n", on ? "ON" : "OFF",
                (unsigned)LED_PIN, on ? "HIGH" : "LOW");
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  char text[16];
  unsigned int n = (length < sizeof(text) - 1) ? length : sizeof(text) - 1;
  memcpy(text, payload, n);
  text[n] = '\0';

  String cmd(text);
  cmd.trim();
  cmd.toUpperCase();
  Serial.printf("[MQTT] received %s : \"%s\"\n", topic, cmd.c_str());

  if (strcmp(topic, TOPIC_LED) != 0) return;

  if (cmd == "ON" || cmd == "1") {
    setLed(true);
  } else if (cmd == "OFF" || cmd == "0") {
    setLed(false);
  } else {
    Serial.printf("[LED] unknown command \"%s\" (use ON or OFF)\n", cmd.c_str());
  }
}

// ============================================================
// Wi-Fi
// ============================================================

void connectWiFi() {
  Serial.printf("[WiFi] connecting to %s ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] connected, IP = %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[WiFi] NOT connected (timeout), will keep retrying in loop()");
  }
}

void keepWiFi() {
  static bool wasConnected = (WiFi.status() == WL_CONNECTED);
  static uint32_t lastTry = 0;

  const bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected && !wasConnected) {
    Serial.printf("[WiFi] reconnected, IP = %s\n", WiFi.localIP().toString().c_str());
  } else if (!connected && wasConnected) {
    Serial.println("[WiFi] connection lost");
  }
  wasConnected = connected;

  if (!connected && millis() - lastTry > 10000) {
    lastTry = millis();
    Serial.println("[WiFi] trying to reconnect ...");
    WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
  }
}

// ============================================================
// MQTT
// ============================================================

void keepMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (mqtt.connected()) {
    mqtt.loop();
    return;
  }

  const uint32_t now = millis();
  if (mqttTried && now - lastMqttTryMs < MQTT_RETRY_MS) return;
  mqttTried = true;
  lastMqttTryMs = now;

  Serial.printf("[MQTT] connecting to %s:%u as %s ...\n", MQTT_HOST, (unsigned)MQTT_PORT, clientId);
  if (mqtt.connect(clientId)) {
    Serial.println("[MQTT] connected");
    if (mqtt.subscribe(TOPIC_LED)) {
      Serial.printf("[MQTT] subscribed to %s\n", TOPIC_LED);
    } else {
      Serial.printf("[MQTT] subscribe FAILED: %s\n", TOPIC_LED);
    }
  } else {
    Serial.printf("[MQTT] connect FAILED, rc=%d (retry in %lu s)\n",
                  mqtt.state(), (unsigned long)(MQTT_RETRY_MS / 1000));
  }
}

// ============================================================
// Cảm biến
// ============================================================

void readSensor() {
  const float h = dht.readHumidity();
  const float t = dht.readTemperature();

  const bool bad = std::isnan(h) || std::isnan(t) || h < 0 || h > 100 || t < -40 || t > 80;
  if (bad) {
    sensorValid = false;
    Serial.println("[DHT22] read FAILED (NaN or out of range), nothing will be published");
    return;
  }

  lastTemperature = t;
  lastHumidity = h;
  sensorValid = true;
  Serial.printf("[DHT22] read OK: temperature = %.1f C, humidity = %.1f %%\n", t, h);
}

void publishSensor() {
  if (!sensorValid) return;
  if (!mqtt.connected()) {
    Serial.println("[MQTT] not connected, skip publish");
    return;
  }

  char payload[64];
  buildSensorJson(payload, sizeof(payload));
  bool ok = mqtt.publish(TOPIC_SENSOR, payload);
  Serial.printf("[MQTT] publish %s : %s -> %s\n", TOPIC_SENSOR, payload, ok ? "OK" : "FAILED");

  char body[192];
  buildApiJson(body, sizeof(body));
  ok = mqtt.publish(TOPIC_API, body);
  Serial.printf("[MQTT] publish %s : %s -> %s\n", TOPIC_API, body, ok ? "OK" : "FAILED");
}

// ============================================================
// setup / loop
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== ESP32 DHT22 -> HTTP /api/sensor + MQTT -> Node-RED (Bai 3, Tuan 4) ===");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.printf("[LED] OFF at boot (GPIO%u = LOW)\n", (unsigned)LED_PIN);

  dht.begin();
  Serial.printf("[DHT22] started on GPIO%u\n", (unsigned)DHT_PIN);

  connectWiFi();

  server.on("/api/sensor", HTTP_GET, handleApiSensor);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[HTTP] server started on port 80, endpoint: GET /api/sensor");

  // Client ID ngẫu nhiên để không trùng với máy khác trên broker công cộng
  snprintf(clientId, sizeof(clientId), "esp32-tnkh-w4-%04lx", (unsigned long)random(0x10000));
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(5);

  // Để vòng loop() đầu tiên đọc cảm biến ngay
  lastSensorMs = millis() - SENSOR_INTERVAL_MS;
}

void loop() {
  server.handleClient();
  keepWiFi();
  keepMqtt();

  const uint32_t now = millis();
  if (now - lastSensorMs >= SENSOR_INTERVAL_MS) {
    lastSensorMs = now;
    readSensor();
    publishSensor();
  }

  delay(10);
}