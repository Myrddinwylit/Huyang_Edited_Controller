//
// Huyang Network Config
//

// Huyang Wifi Modes
// WifiModeHotspot = creates his own wifi with the following Data
// WifiModeNetwork = connects to a existing wifi with an existing password
// #define WiFiDefaultMode JxWifiManager::WifiModeHotspot
#define WiFiDefaultMode JxWifiManager::WifiModeNetwork

// To get this running, change the WifiSsid to whatever your wifi should be called
// this Wifi will be created by Huyang himself. Don' name this like your home Wifi.
#define WifiSsidOfHotspot "HuyangWifiControl" // <- change if needed

// set a custom passwort so not everyone can control your Huyang Droid
// The password has a minimum length of 8 symbols or "" if you dont want a password.
#define WifiPasswordHotspot "" // <- change if needed

// Connect to your local Wifi. If not Accessible Huyang will create Hotspot
#define WifiSsidConnectTo "Yavin4"         // <- change if needed
// The password has a minimum length of 8 symbols or "" if you dont want a password.
#define WifiPasswordConnectTo "29833170985536833475" // <- change if needed


// ONLY if the Huyang Wifi Mode is Mode WifiModeNetwork:
// Connect to your Wifi Router and check the connected Devices to get his IP Adress
// When connected via USB, the console will write down the current IP Adress every 5 seconds.

// Webserver Port default is 80. If you want a different Port, change it
#define WebServerPort 80

// System Option
// Here you can enable/disable some sub-sections of the software to match your build
// To disable an option, change: true; to false;

// The following are now 'extern' declarations. Their definitions are in Huyang_Remote_Control.ino.
extern bool enableEyes;            // the two TFT Displays as eyes
extern bool enableMonacle;         // the servo inside of the head shell to move the monacle
extern bool enableNeckMovement;    // the two servos inside of the head
extern bool enableHeadRotation;    // the rotation servo on the bottom of the neck
extern bool enableBodyMovement;    // the four 60kg servos from the c-3po build plan
extern bool enableBodyRotation;    // the 80kg servo inside of the hip
extern bool enableTorsoLights;     // the ws2812b led inside of the torso
