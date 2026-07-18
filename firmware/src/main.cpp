#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>

#include "../include/secrets.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void showText(String text) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(text);
    display.display();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        while (1);
    }

    showText("Connecting WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi FAILED to connect");
        showText("WiFi FAILED");
        return;
    }

    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    showText("Asking...");

    HTTPClient https;
    https.begin("http://" + String(serverIp) + ":5000/ask");
    https.addHeader("X-Device-Secret", deviceSecret);
    https.addHeader("content-type", "application/json");

    JsonDocument doc;
    doc["message"] = "Hello!";
    String requestBody;
    serializeJson(doc, requestBody);

    int httpCode = https.POST(requestBody);

    if (httpCode == 200) {
        String response = https.getString();
        JsonDocument responseDoc;
        deserializeJson(responseDoc, response);
        const char *reply = responseDoc["reply"];

        Serial.println(reply);
        showText(String(reply));
    } else {
        Serial.printf("Ошибка: %d\n", httpCode);
        Serial.println(https.getString());
        showText("Error: " + String(httpCode));
    }

    https.end();
}

void loop() {}
