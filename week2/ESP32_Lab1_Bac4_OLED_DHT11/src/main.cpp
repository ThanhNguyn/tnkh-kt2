#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =========================
// Pin configuration - Lab 1
// =========================
constexpr uint8_t BUTTON_PIN = 18;
constexpr uint8_t LED1_PIN = 4;
constexpr uint8_t DHT_PIN = 27;

constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;

constexpr uint32_t DEBOUNCE_MS = 50;
constexpr uint32_t DHT_INTERVAL_MS = 2000;

// =========================
// OLED
// =========================
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);

// =========================
// DHT
// Wokwi dùng DHT22 model.
// Kit thật của Lab 1: đổi DHT22 -> DHT11.
// =========================
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// =========================
// Button ISR + debounce
// =========================
volatile uint32_t nhan = 0;
volatile uint32_t tLan = 0;

void IRAM_ATTR onNut() {
    uint32_t t = millis();

    if (t - tLan > DEBOUNCE_MS) {
        nhan = nhan + 1;
        tLan = t;
    }
}

// =========================
// State
// =========================
uint32_t daIn = 0;
uint32_t lastDhtRead = 0;

// =========================
// I2C scanner
// =========================
void scanI2C() {
    Serial.println();
    Serial.println("=== I2C SCANNER ===");

    uint8_t found = 0;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf(
                "Found I2C device at 0x%02X\n",
                address
            );
            found++;
        }
    }

    if (found == 0) {
        Serial.println("No I2C device found.");
    } else {
        Serial.printf(
            "Total devices found: %u\n",
            found
        );
    }

    Serial.println("====================");
    Serial.println();
}

// =========================
// OLED DHT display
// =========================
void showDhtOnOLED() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    display.clearDisplay();

    // Header
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("TNKHKT2 Lab1 - Nhom X");

    if (isnan(humidity) || isnan(temperature)) {
        display.setTextSize(2);
        display.setCursor(0, 22);
        display.println("DHT loi");
        display.setTextSize(1);
        display.setCursor(0, 50);
        display.println("Kiem tra day");
        display.display();

        Serial.println("DHT loi - kiem tra day");
        return;
    }

    // Temperature
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.printf("T: %.1f C", temperature);

    // Humidity
    display.setCursor(0, 42);
    display.printf("H: %.1f %%", humidity);

    display.display();

    Serial.printf(
        "DHT: temperature=%.1f C, humidity=%.1f %%\n",
        temperature,
        humidity
    );
}

// =========================
// Setup
// =========================
void setup() {
    Serial.begin(115200);

    // Button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        onNut,
        FALLING
    );

    // LEDC - Bậc 2 vẫn hoạt động
    ledcAttach(LED1_PIN, 5000, 8);
    ledcWrite(LED1_PIN, 0);

    // I2C: không gọi Wire.begin() ở đây vì Adafruit_SSD1306::begin()
    // đã tự xử lý khởi tạo bus I2C. Việc khởi tạo lặp hoặc quét bus
    // trước khi OLED ready sẽ làm ESP32 rơi vào ESP_ERR_INVALID_STATE.

    Serial.println();
    Serial.println("=================================");
    Serial.println("LAB 1 - BAC 4");
    Serial.println("ESP32 DevKit v1");
    Serial.println("Button : GPIO18");
    Serial.println("LED1   : GPIO4 / LEDC");
    Serial.println("DHT    : GPIO27");
    Serial.println("OLED   : SDA21 / SCL22 / 0x3C");
    Serial.println("=================================");

    // Không quét I2C ở đây: việc gọi beginTransmission() lặp lại trên bus
    // đang hoạt động có thể làm ESP32 rơi vào trạng thái INVALID_STATE.
    // OLED sẽ tự khởi tạo trên địa chỉ 0x3C.

    // OLED
    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDRESS)) {

        Serial.println("OLED init failed.");
    } else {
        Serial.println("OLED initialized at 0x3C.");
    }

    // DHT
    dht.begin();

    // Cho phép đọc lần đầu ngay lập tức
    lastDhtRead = 0;
}

// =========================
// Loop
// =========================
void loop() {
    // ---------------------------------
    // Bậc 2: button -> LEDC
    // ---------------------------------
    if (nhan != daIn) {
        daIn = nhan;

        uint8_t brightness =
            (nhan % 8) * 32;

        ledcWrite(
            LED1_PIN,
            brightness
        );

        Serial.printf(
            "nhan=%lu | brightness=%u/255\n",
            static_cast<unsigned long>(nhan),
            brightness
        );
    }

    // ---------------------------------
    // Bậc 4: DHT11/DHT22 mỗi 2 giây
    // ---------------------------------
    uint32_t now = millis();

    if (now - lastDhtRead >= DHT_INTERVAL_MS ||
        lastDhtRead == 0) {

        lastDhtRead = now;

        showDhtOnOLED();
    }
}