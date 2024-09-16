# Referee Schedule Automation with ESP32

## Introduction

This project automates the process of retrieving and managing referee schedules from two sources: **RefCentre** and **COMET**. It fetches schedules from these sources, stores them locally, serves them via an HTTP server, and provides access to the schedules through a hardware-based user interface implemented on an **ESP32** using an LCD display and a rotary encoder for date selection.

## Features
- Automated login to **RefCentre** and **COMET** to scrape schedules.
- Files are saved in a structured format by date in a local directory.
- Abbreviations for leagues and fields are applied to make the data easier to read.
- The data is served locally via an HTTP server on port `8000`.
- A separate **ESP32**-based hardware system provides a UI for viewing schedules with a date selector.

## Installation

### Python Script
1. Install Python 3.x if not already installed.
2. Install necessary Python libraries 
3. Install Playwright dependencies:
4. Clone or download the project.

### ESP32 Code
1. Install the Arduino IDE or PlatformIO to program the ESP32.
2. Download and install the required libraries.
3. Upload the ESP32 code to your device.

## Usage
### Running the Python Script
1. Run the script by executing "python webscrape.py"
2. Enter your RefCentre and COMET credentials when prompted.
3. The script will:
  - Fetch schedules from both RefCentre and COMET.
  - Save the schedules as .txt files in the local directory.
  - Start an HTTP server that serves the schedules.
4. You can access the schedule files by navigating to http://localhost:8000 or from another device on the same network using the IP address of the server.
### Using the ESP32 Interface
1. Upon boot, the ESP32 will connect to Wi-Fi and display the current date.
2. Use the rotary encoder to select different dates and view the corresponding schedule fetched from the HTTP server.
3. Press the button to confirm your selection.
4. Scroll through games using rotary encoder.

## Dependencies
### Python Dependencies
  -playwright
  -beautifulsoup4
  -datetime
  -os
  -re
  -threading
  -time
### ESP32 Dependencies
  -Wire.h (for I2C communication with LCD)
  -LiquidCrystal_I2C.h (for LCD display)
  -WiFi.h (for Wi-Fi connection)
  -HTTPClient.h (for HTTP requests)
## Configuration
### Pyhthon Configuration
The script fetches schedules from RefCentre and COMET by logging in with the credentials you provide. It saves schedules as .txt files in the C:\Ref Games directory and serves them on http://0.0.0.0:8000.

You can modify the location where the schedules are saved by editing the path variable in the main() function.

### ESP32 Configuration
You will need to configure your Wi-Fi credentials within the ESP32 code:

  - const char* ssid = "YOUR_SSID";
  - const char* password = "YOUR_WIFI_PASSWORD";

### ESP32 Integration

  - The ESP32 code interfaces with the Python backend using an HTTP client. When a date is selected on the LCD, the ESP32 fetches the corresponding schedule from the server and displays it on the screen.
  - The rotary encoder allows you to scroll through dates.
  - A button press confirms the selection of a date, triggering an HTTP request to the server to fetch the schedule for that date.
  
  
### Schematic
  - LCD Display: Connected via I2C to the ESP32.  
  - Rotary Encoder: Connected to digital pins for reading pulses from the encoder.  
  - Button: Used to select dates and reset selections.
