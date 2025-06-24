
#ifndef JxWifi_h
#define JxWifi_h

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

class JxWifiManager
{
public:
    JxWifiManager(bool debug = false);

    enum WifiMode
    {
        WifiModeHotspot,
        WifiModeNetwork
    };

    // Wifi Settings
    WifiMode currentMode = WifiModeNetwork;

    IPAddress host;
    IPAddress subnetMask;

    // Hotspot
    String hotspot_Ssid;
    String hotspot_Password;

    // use Local Wifi
    String network_Ssid;
    String network_Password;

    bool isConnected();
    void setup();
    void loop();

    IPAddress getCurrentIPAdress();

private:
    bool _debug = false;
    uint8_t _tryCount = 0;
    unsigned long _lastTry = 0;
};

#endif