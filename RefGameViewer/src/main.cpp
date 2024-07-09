#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <time.h>

void handleDial();
void getCurrTime();
void printLocalTime();
void showDates(int, int);

#define green 4
#define clk 17
#define data 16
#define swtch 0 

int lastCLKState;
volatile int encoderValue = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid = "Patroifam_2.4GHz";
const char* password = "Oakville";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5*3600;
const int daylightOffset_sec = 3600;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(swtch, INPUT_PULLUP);
    pinMode(green, OUTPUT);
    pinMode(clk, INPUT);
    pinMode(data, INPUT);
    lastCLKState = digitalRead(clk);
    lcd.init();
    lcd.backlight();

    attachInterrupt(digitalPinToInterrupt(clk), handleDial, CHANGE);

    Serial.begin(921600);
    lcd.setCursor(0, 0);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        lcd.print(".");
    }
    lcd.print("Connected to WiFi");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    getCurrTime();
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000) {
        getCurrTime();
        lastUpdate = millis();
    }
    lcd.setCursor(0, 1);
    lcd.print(encoderValue);
}

void showDates(day, month)
{

}


void handleDial() {
    int CLKState = digitalRead(clk);
    int DTState = digitalRead(data);

    if (CLKState != lastCLKState && CLKState == HIGH) {
        if (DTState != CLKState) {
            encoderValue++;
        } else {
            encoderValue--;
        }
    }

    lastCLKState = CLKState;
}

void getCurrTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        lcd.print("Failed to obtain time");
        return;
    }
    printLocalTime();
}

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &timeinfo);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(buffer);
}
