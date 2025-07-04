# Huyang Remote Control
THIS REPOSITORY HAS NOT BEEN FULLY TESTED PLEASE USE CAUTION IF YOU FOUND THIS


This Software was Developed by Jeanette Müller - this is a modified version of her software to suit some of my needs I do not assume claim or ownership of her work - i have used her code as a baseline for devleoping some other features for my Professor Huyang build.


Software for ESP8266 to control Huyang from DroidDivision via Browser/Mobile Device.
This Software runs on mostly any ESP8266 together with two GC9A01 TFT Displays. 
Inside the Folder /Wiring Diagrams you can find some Instructions how to connect two GC9A01 Displays to common Boards. 

![Preview of 1.9](img/1_9.png)

# HOW TO Get this running!!!
Open "config.h" and edit the configuration as you need it.
You can configure Huyang to be accessible via his own Wifi or let him connect to your home WiFi. 

# Install Board Library
1. Open "Settings"
2. Edit the "Additional boards manager URLs"
3. Add new Row: "http://arduino.esp8266.com/stable/package_esp8266com_index.json"
4. Press OK
5. Restart Arduino IDE
6. Open "Tools" -> "Board ..." -> "Board Manager"
7. Search for "esp"
8. Install "esp32 by Espressif" OR "ESP8266 Boards (3.1.0)" to make your Board available
9. Select "Tools" -> "Board" -> ... whatever you have. Maybe "Generic ESP8266 Module", or "ESP32 Dev Modul"



# Install Software Libraries
For all installations, if there are some dependencies to install, install them!

1. Open "Tools" -> "Manage Libraries"
2. Search for following Names and install what you found
3. "ESPAsyncWebServer" -> Install "ESPAsyncWebServer by Iacamera"
4. "PWM Servo Driver" -> Install "Adafruit PWM Servo Driver Library by Adafruit"
5. "Adafruit NeoPixel" -> Install "Adafruit NeoPixel by Adafruit"
6. "Arduino GFX Library" -> Install "GFX Library for Arduino by Moon On Our Nation"
7. "DFRobotDFPlayerMini" -> Install "DFRobotDFPlayerMini by DFRobot"
8. "ArduinoJson" -> Install "ArduinoJson by Bernoit Blanchon"

# Install Arduino IDE 2 Plugin
1. Please install "arduino-littlefs-upload-1.5.0.vsix" by following the Instructions of https://randomnerdtutorials.com/arduino-ide-2-install-esp8266-littlefs/#installing

# Run
* Build and Run with your Arduino IDE
* Huyang Communicate via Serial (USB) on Port 115200

## using local WiFi
* On the Serial Monitor you can read the current IP Adress when in WifiMode Connect to your Wifi
* Open a Browser of a Device connected to the same Wifi
* Enter the IP Adress from the Serial Monitor into your Browser Adressbar 
* This looks like http://192.168.10.1 or if WebServerPort is not 80, try http://192.168.10.1:80

## using hotspot
* Enter http://192.168.10.1 into your Browser Adressbar 
* If you changed the WebServerPort, try http://192.168.10.1:80 and replace the :80 with your custom port (like :123)

# Changelog

[Changelog](changelog.md)
