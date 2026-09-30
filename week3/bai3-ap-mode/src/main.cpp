#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

constexpr uint8_t LED_PIN = 2;

const char* AP_SSID = "ThanhESP32";
const char* AP_PASSWORD = "Thanh@2026";

#ifdef WOKWI_SIM
const char* WOKWI_SSID = "Wokwi-GUEST";
const char* WOKWI_PASSWORD = "";

constexpr uint32_t STA_TIMEOUT_MS = 10000;
#endif

WebServer server(80);

bool ledState = false;

String buildWebPage() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta
        name="viewport"
        content="width=device-width, initial-scale=1.0"
    >

    <title>ESP32 AP Control</title>

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

        .card {
            width: min(100%, 460px);

            padding: 32px;
            background: white;
            border-radius: 20px;

            text-align: center;

            box-shadow: 0 8px 30px rgba(0, 0, 0, 0.10);
        }

        h1 {
            margin: 0 0 8px;
        }

        .subtitle {
            color: #6b7280;
            margin-bottom: 24px;
        }

        .network {
            margin-bottom: 24px;
            padding: 16px;

            background: #f9fafb;
            border-radius: 12px;

            text-align: left;
            line-height: 1.8;
        }

        .state {
            margin: 24px 0;

            font-size: 24px;
            font-weight: bold;
        }

        .buttons {
            display: flex;
            flex-wrap: wrap;

            justify-content: center;
            gap: 10px;
        }

        button {
            border: none;
            border-radius: 10px;

            padding: 14px 24px;

            font-size: 16px;
            cursor: pointer;
        }

        button:active {
            transform: scale(0.97);
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
        <h1>ESP32 AP Control</h1>

        <div class="subtitle">
            Access Point Mode
        </div>

        <div class="network">
            <div>
                <strong>SSID:</strong>
                ThanhESP32
            </div>

            <div>
                <strong>Password:</strong>
                Thanh@2026
            </div>

            <div>
                <strong>AP IP:</strong>
                192.168.4.1
            </div>
        </div>

        <div class="state">
            LED:
            <span id="ledState">%STATE%</span>
        </div>

        <div class="buttons">
            <button
                class="on"
                onclick="setLED('/on')"
            >
                ON
            </button>

            <button
                class="off"
                onclick="setLED('/off')"
            >
                OFF
            </button>

            <button
                class="toggle"
                onclick="setLED('/toggle')"
            >
                TOGGLE
            </button>
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
    </script>
</body>
</html>
)rawliteral";

    html.replace(
        "%STATE%",
        ledState ? "ON" : "OFF"
    );

    return html;
}

void handleRoot() {
    server.send(
        200,
        "text/html",
        buildWebPage()
    );
}

void handleOn() {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);

    server.send(
        200,
        "text/plain",
        "ON"
    );
}

void handleOff() {
    ledState = false;
    digitalWrite(LED_PIN, LOW);

    server.send(
        200,
        "text/plain",
        "OFF"
    );
}

void handleToggle() {
    ledState = !ledState;

    digitalWrite(
        LED_PIN,
        ledState ? HIGH : LOW
    );

    server.send(
        200,
        "text/plain",
        ledState ? "ON" : "OFF"
    );
}

void handleNotFound() {
    server.send(
        404,
        "text/plain",
        "404 - Not Found"
    );
}

void startAccessPoint() {
    WiFi.mode(
#ifdef WOKWI_SIM
        WIFI_AP_STA
#else
        WIFI_AP
#endif
    );

    bool apStarted = WiFi.softAP(
        AP_SSID,
        AP_PASSWORD
    );

    if (!apStarted) {
        Serial.println("Failed to start Access Point!");
        return;
    }

    Serial.println();
    Serial.println("Access Point started!");

    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);

    Serial.print("AP Password: ");
    Serial.println(AP_PASSWORD);

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

#ifdef WOKWI_SIM

void connectWokwiSTA() {
    Serial.println();
    Serial.print("Connecting STA to ");
    Serial.println(WOKWI_SSID);

    WiFi.begin(
        WOKWI_SSID,
        WOKWI_PASSWORD,
        6
    );

    const uint32_t startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < STA_TIMEOUT_MS
    ) {
        delay(250);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("STA connected!");

        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("STA connection timeout.");
    }
}

#endif

void startWebServer() {
    server.on(
        "/",
        HTTP_GET,
        handleRoot
    );

    server.on(
        "/on",
        HTTP_GET,
        handleOn
    );

    server.on(
        "/off",
        HTTP_GET,
        handleOff
    );

    server.on(
        "/toggle",
        HTTP_GET,
        handleToggle
    );

    server.onNotFound(
        handleNotFound
    );

    server.begin();

    Serial.println();
    Serial.println("HTTP server started!");
}

void setup() {
    Serial.begin(115200);

    pinMode(
        LED_PIN,
        OUTPUT
    );

    digitalWrite(
        LED_PIN,
        LOW
    );

    startAccessPoint();

#ifdef WOKWI_SIM
    connectWokwiSTA();
#endif

    startWebServer();
}

void loop() {
    server.handleClient();
}