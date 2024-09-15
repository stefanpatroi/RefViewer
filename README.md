RefViewer
This project integrates an ESP32 microcontroller with an LCD display to show game schedules fetched from two different platforms: RefCentre and COMET. The device allows users to select a date using a rotary encoder, retrieve game data from a local server based on the selected date, and display the data on the LCD screen. The project also includes a Python script for web scraping game schedules from RefCentre and COMET, storing them as text files on a local server.

Features:
ESP32-Based Interface: Displays game schedules on an LCD screen.
Rotary Encoder: Allows users to navigate between dates to select a specific schedule.
Web Scraping: Automated scraping of game schedules from RefCentre and COMET using Python.
Local Server: A Python-based HTTP server provides the game data to the ESP32.
Abbreviation Replacement: Common locations and leagues are shortened for display clarity.
Line Scrolling: If game data exceeds the display limit, a dial allows scrolling through the lines.
Components Used:
ESP32 microcontroller
LiquidCrystal I2C (20x4) LCD
Rotary encoder (with push-button)
Push Button for Reset Functionality
Python for web scraping and HTTP server
Wi-Fi for ESP32 connectivity
