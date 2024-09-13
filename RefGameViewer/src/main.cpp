#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <time.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

void handleDial();
void getInitialDate(char* dateBuffer);
void toggleDates();
void updateDisplay();
void selectDate();
void extractTextFromHTML(const String& html);

#define dateReset 4
#define clk 17
#define data 16
#define swtch 0

boolean selectionMade = false; 
int lastCLKState;
int selectedDay;
int selectedMonth;
int selectedYear;
int totalLines = 0;
char initialDate[11];
char selectedDate[11];
volatile int encoderValue = 0;
volatile int displayStartLine = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);
String httpText; 
String* extractedLines = new String[1];

const char* ssid = "";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600;
const int daylightOffset_sec = 3600;

void setup()
{
    pinMode(dateReset, INPUT_PULLUP);
    pinMode(swtch, INPUT_PULLUP);
    pinMode(clk, INPUT);
    pinMode(data, INPUT);
    lastCLKState = digitalRead(clk);
    lcd.init();
    lcd.backlight();

    attachInterrupt(digitalPinToInterrupt(clk), handleDial, CHANGE);

    Serial.begin(921600);
    lcd.setCursor(0, 0);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        lcd.print(".");
    }
    lcd.setCursor(0, 0);
    lcd.print("Connected to WiFi");
    delay(1000);
    lcd.clear();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    getInitialDate(initialDate);
    lcd.print("Start: Press Button");
    while (digitalRead(dateReset) == HIGH)
    {
        
    }
    updateDisplay();
}

void loop()
{
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 150)
    {
        updateDisplay();
        lastUpdate = millis();
        if (digitalRead(dateReset) == LOW)
        {
            encoderValue = 0;
        }
    }
}

void toggleDates()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }

    timeinfo.tm_mday += encoderValue;
    mktime(&timeinfo);

    selectedDay = timeinfo.tm_mday;
    selectedMonth = timeinfo.tm_mon + 1;
    selectedYear = timeinfo.tm_year + 1900;

    strftime(selectedDate, 11, "%d-%m-%y", &timeinfo);
    lcd.setCursor(0, 0);
    lcd.print("Today: ");
    lcd.print(initialDate);
    lcd.setCursor(0, 1);
    lcd.print("Selection: ");
    lcd.setCursor(0, 2);
    lcd.print(selectedDate);
    lcd.setCursor(0, 3);
    lcd.print("Reset: Press Button");
}

void handleDial()
{
    int CLKState = digitalRead(clk);
    int DTState = digitalRead(data);

    if (CLKState != lastCLKState && CLKState == HIGH)
    {
        if (DTState != CLKState)
        {
            encoderValue++;
            if ((displayStartLine < totalLines - 4) && selectionMade == true)
            {
                displayStartLine++;
            }
        }
        else
        {
            if (encoderValue > 0)
            {
                encoderValue--;
                if ((displayStartLine > 0) && selectionMade == true)
                {
                    displayStartLine--;
                }
            }
        }
    }
    encoderValue = encoderValue % 15;
    lastCLKState = CLKState;
}

void getInitialDate(char* dateBuffer)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        dateBuffer[0] = '\0';
        return;
    }
    strftime(dateBuffer, 11, "%d-%m-%y", &timeinfo);
}

void selectDate()
{
    if (digitalRead(swtch) == LOW)
    {
        lcd.clear();
        selectionMade = true;
        lcd.setCursor(0, 0);
        lcd.print("Chosen Date:");
        lcd.setCursor(0, 1);
        lcd.print(selectedDate);
        lcd.setCursor(0, 2);
        lcd.print("To select new date");
        lcd.setCursor(0, 3);
        lcd.print("Press Reset Button ");
        delay(2000);
        
        WiFiClient client;
        HTTPClient http;

        String serverPath = "http://192.168.1.42:8000/" + String(selectedDate) +".txt";
        http.begin(client, serverPath);
        
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            String htmlContent = http.getString();
            Serial.print(htmlContent);
            Serial.print("THATS THE CONTENT");
            http.end();
            Serial.print("Before");
            extractTextFromHTML(htmlContent);
            Serial.print("After");
        }
        else
        {
            httpText = "Error: " + String(httpResponseCode);
        }
        
        http.end();
    }
}

void updateDisplay()
{
    lcd.clear();
    if (!selectionMade)
    {
        toggleDates();
    }
    selectDate();
    if (selectionMade)
    {
        if (totalLines < 4)
        {
            displayStartLine = 0; 
        }
        else
        {
            displayStartLine = constrain(displayStartLine, 0, totalLines - 4);
        }

        if (extractedLines != nullptr) 
        {
            for (int i = 0; i < 4; i++)
            {
                if (displayStartLine + i < totalLines)
                {
                    lcd.setCursor(0, i);
                    lcd.print(extractedLines[displayStartLine + i].substring(0, 20));
                }
                else
                {
                    lcd.setCursor(0, i);
                    lcd.print("END OF GAMES");
                }
            }
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("Error: No Data");
        }
        
        if (digitalRead(dateReset) == LOW)
        {
            selectionMade = false;
        }
    }
}


void extractTextFromHTML(const String& html)
{
    int start = 0;
    int end = 0;
    totalLines = 0;

    while ((end = html.indexOf('\n', start)) != -1)
    {
        totalLines++;
        start = end + 1;
    }

    if (start < html.length())
    {
        totalLines++;
    }

    if (extractedLines != nullptr)
    {
        delete[] extractedLines;
    }

    if (totalLines > 0)
    {
        extractedLines = new String[totalLines];
    }
    else
    {
        extractedLines = new String[1];
        extractedLines[0] = "No Data";
        totalLines = 1;
    }

    start = 0;
    int currLine = 0;
    while ((end = html.indexOf('\n', start)) != -1 && currLine < totalLines)
    {
        String lineText = html.substring(start, end);
        lineText.trim();
        if (lineText.length() > 0)
        {
            extractedLines[currLine++] = lineText;
        }
        start = end + 1;
    }

    if (start < html.length() && currLine < totalLines)
    {
        String lineText = html.substring(start);
        lineText.trim();
        if (lineText.length() > 0)
        {
            extractedLines[currLine++] = lineText;
        }
    }
}


