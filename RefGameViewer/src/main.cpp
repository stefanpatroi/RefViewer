#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <time.h>

void handleDial();
void getInitialDate(char* dateBuffer);
void toggleDates();
void updateDisplay();

#define green 4
#define clk 17
#define data 16
#define swtch 0 

int lastCLKState;
int selectedDay;
int selectedMonth;
int selectedYear;
char initialDate[11];
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
    lcd.setCursor(0, 0);
    lcd.print("Connected to WiFi");
    delay(1000);
    lcd.clear();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    getInitialDate(initialDate);
    updateDisplay();
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 100) {
        updateDisplay();
        lastUpdate = millis();
    }
}

void toggleDates()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    timeinfo.tm_mday += encoderValue;
    mktime(&timeinfo); 

    selectedDay = timeinfo.tm_mday;
    selectedMonth = timeinfo.tm_mon + 1; 
    selectedYear = timeinfo.tm_year + 1900; 

    char dateBuffer[11];
    strftime(dateBuffer, 11, "%d-%m-%Y", &timeinfo);
    lcd.setCursor(0, 2);
    lcd.print(dateBuffer);
}

void handleDial() {
    int CLKState = digitalRead(clk);
    int DTState = digitalRead(data);

    if (CLKState != lastCLKState && CLKState == HIGH) 
    {
        if (DTState != CLKState) 
        {
            encoderValue++;
        } 
        else 
        {
           if(encoderValue>0)
           {
             encoderValue--;
           } 
        }
    }


    lastCLKState = CLKState;
}

void getInitialDate(char* dateBuffer) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        dateBuffer[0] = '\0'; 
        return;
    }

    // selectedDay = timeinfo.tm_mday;
    // selectedMonth = timeinfo.tm_mon + 1; 
    // selectedYear = timeinfo.tm_year + 1900;
    strftime(dateBuffer, 11, "%d-%m-%Y", &timeinfo);
}

void updateDisplay() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Today: ");
    lcd.print(initialDate);
    lcd.setCursor(0, 1);
    lcd.print("Selection: ");
    toggleDates();
}
