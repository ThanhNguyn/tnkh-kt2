#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

constexpr uint8_t LED_PIN = 2;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

WebServer server(80);

bool ledState = false;

String buildWebPage() {
    String state = ledState ? "ON" : "OFF";

    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 LED Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f3f4f6;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
        }

        .card {
            width: min(90%, 420px);
            background: white;
            padding: 32px;
            border-radius: 18px;
            box-shadow: 0 8px 30px rgba(0, 0, 0, 0.10);
            text-align: center;
        }

        h1 {
            margin-top: 0;
        }

        .state {
            font-size: 24px;
            font-weight: bold;
            margin: 24px 0;
        }

        button {
            border: none;
            border-radius: 10px;
            padding: 14px 24px;
            margin: 5px;
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
    </style>
</head>

<body>
    <div class="card">
        <h1>ESP32 LED Control</h1>

        <div class="state">
            LED: %STATE%
        </div>

        <a href="/on">
            <button class="on">ON</button>
        </a>

        <a href="/off">
            <button class="off">OFF</button>
        </a>

        <a href="/toggle">
            <button class="toggle">TOGGLE</button>
        </a>
    </div>
</body>
</html>
)rawliteral";

    html.replace("%STATE%", state);

    return html;
}

void handleRoot() {
    server.send(200, "text/html", buildWebPage());
}

void handleOn() {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleOff() {
    ledState = false;
    digitalWrite(LED_PIN, LOW);

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleToggle() {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);

    server.sendHeader("Location", "/");
    server.send(303);
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

    connectWiFi();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/on", HTTP_GET, handleOn);
    server.on("/off", HTTP_GET, handleOff);
    server.on("/toggle", HTTP_GET, handleToggle);
    server.onNotFound(handleNotFound);

    server.begin();

    Serial.println("HTTP server started!");
    Serial.println("Open http://localhost:8180 in your browser.");
}

void loop() {
    server.handleClient();
}