#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHTesp.h>

constexpr uint8_t LED_PIN = 2;
constexpr uint8_t DHT_PIN = 15;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

WebServer server(80);
DHTesp dht;

bool ledState = false;

float temperature = NAN;
float humidity = NAN;

unsigned long lastSensorRead = 0;
constexpr unsigned long SENSOR_INTERVAL = 2500;

void readSensor() {
    if (millis() - lastSensorRead < SENSOR_INTERVAL) {
        return;
    }

    lastSensorRead = millis();

    TempAndHumidity data = dht.getTempAndHumidity();

    if (dht.getStatus() != 0) {
        Serial.print("DHT error: ");
        Serial.println(dht.getStatusString());
        return;
    }

    temperature = data.temperature;
    humidity = data.humidity;

    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
}

String buildWebPage() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 IoT Dashboard</title>

    <style>
        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            padding: 24px;
            font-family: Arial, sans-serif;
            background: #f3f4f6;
            display: flex;
            justify-content: center;
            align-items: center;
        }

        .container {
            width: min(100%, 520px);
        }

        .card {
            background: white;
            padding: 28px;
            border-radius: 20px;
            box-shadow: 0 8px 30px rgba(0, 0, 0, 0.10);
            margin-bottom: 18px;
        }

        h1 {
            margin: 0 0 8px;
            text-align: center;
        }

        .subtitle {
            text-align: center;
            color: #6b7280;
            margin-bottom: 24px;
        }

        .status {
            text-align: center;
            font-size: 24px;
            font-weight: bold;
            margin: 20px 0;
        }

        .buttons {
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            gap: 10px;
        }

        button {
            border: none;
            border-radius: 10px;
            padding: 13px 24px;
            font-size: 16px;
            cursor: pointer;
        }

        .on {
            background: #22c55e;
            color: white;
        }

        .off {
            background: #ef4444;
            color: white;
        }

        .toggle {
            background: #374151;
            color: white;
        }

        .sensor-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 14px;
        }

        .sensor {
            padding: 20px;
            border-radius: 14px;
            background: #f9fafb;
            text-align: center;
        }

        .label {
            color: #6b7280;
            margin-bottom: 8px;
        }

        .value {
            font-size: 28px;
            font-weight: bold;
        }
    </style>
</head>

<body>
    <div class="container">

        <div class="card">
            <h1>ESP32 IoT Dashboard</h1>
            <div class="subtitle">LED Control</div>

            <div class="status">
                LED: <span id="ledState">%LED_STATE%</span>
            </div>

            <div class="buttons">
                <button class="on" onclick="setLED('/on')">ON</button>
                <button class="off" onclick="setLED('/off')">OFF</button>
                <button class="toggle" onclick="setLED('/toggle')">TOGGLE</button>
            </div>
        </div>

        <div class="card">
            <h2>Environment</h2>

            <div class="sensor-grid">
                <div class="sensor">
                    <div class="label">Temperature</div>
                    <div class="value">
                        <span id="temperature">--</span> °C
                    </div>
                </div>

                <div class="sensor">
                    <div class="label">Humidity</div>
                    <div class="value">
                        <span id="humidity">--</span> %
                    </div>
                </div>
            </div>
        </div>

    </div>

    <script>
        async function setLED(endpoint) {
            try {
                const response = await fetch(endpoint);
                const state = await response.text();
                document.getElementById("ledState").textContent = state;
            } catch (error) {
                console.error(error);
            }
        }

        async function updateSensor() {
            try {
                const response = await fetch("/sensor");
                const data = await response.json();

                if (data.temperature !== null) {
                    document.getElementById("temperature").textContent =
                        data.temperature.toFixed(1);
                }

                if (data.humidity !== null) {
                    document.getElementById("humidity").textContent =
                        data.humidity.toFixed(1);
                }
            } catch (error) {
                console.error(error);
            }
        }

        updateSensor();
        setInterval(updateSensor, 2500);
    </script>
</body>
</html>
)rawliteral";

    html.replace("%LED_STATE%", ledState ? "ON" : "OFF");

    return html;
}

void handleRoot() {
    server.send(200, "text/html", buildWebPage());
}

void handleOn() {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    server.send(200, "text/plain", "ON");
}

void handleOff() {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    server.send(200, "text/plain", "OFF");
}

void handleToggle() {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    server.send(200, "text/plain", ledState ? "ON" : "OFF");
}

void handleSensor() {
    readSensor();

    String json = "{";
    json += "\"temperature\":";
    json += isnan(temperature) ? "null" : String(temperature, 1);
    json += ",";
    json += "\"humidity\":";
    json += isnan(humidity) ? "null" : String(humidity, 1);
    json += "}";

    server.send(200, "application/json", json);
}

void handleNotFound() {
    server.send(404, "text/plain", "404 - Not Found");
}

void connectWiFi() {
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    dht.setup(DHT_PIN, DHTesp::DHT22);

    connectWiFi();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/on", HTTP_GET, handleOn);
    server.on("/off", HTTP_GET, handleOff);
    server.on("/toggle", HTTP_GET, handleToggle);
    server.on("/sensor", HTTP_GET, handleSensor);
    server.onNotFound(handleNotFound);

    server.begin();

    Serial.println("HTTP server started!");

    delay(2500);
    readSensor();
}

void loop() {
    server.handleClient();
    readSensor();
}