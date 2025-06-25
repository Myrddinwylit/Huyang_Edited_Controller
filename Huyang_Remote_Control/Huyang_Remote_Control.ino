// HOW TO Get this running!!!
// open config.h and edit the configuration as you need it

// Include necessary headers for declarations.
// These headers tell the compiler what variables/objects exist, but don't create them yet.
#include "src/includes.h"    // Contains other standard Arduino libraries and class header includes
#include "config.h"          // Contains extern declarations for feature flags
#include "calibration.h"     // Contains extern declaration for calibrations array
#include "src/definitions.h" // Contains extern declarations for global objects like wifi, pwm, huyangFace, etc.

// --- GLOBAL DEFINITIONS (allocate memory for extern-declared variables/objects) ---
// These variables and objects are now defined *once* here.
// This resolves the "multiple definition" linker errors that occur when they are defined
// directly within header files that are included multiple times.

// From config.h (feature flags):
// These should be defined here, as config.h only provides extern declarations.
bool enableEyes = true;
bool enableMonacle = true;
bool enableNeckMovement = true;
bool enableHeadRotation = true;
bool enableBodyMovement = true;
bool enableBodyRotation = true;
bool enableTorsoLights = true;

// From calibration.h (calibration data):
// This array holds your calibration data as a JSON string.
char calibrations[] = "{\"neck\":{\"rotation\":0,\"tiltForward\":0,\"tiltSideways\":0},\"body\":{\"rotation\":0,\"tiltForward\":0,\"tiltSideways\":0}}";


// From definitions.h (global objects and time variables):
unsigned long currentMillis = 0;
unsigned long previousMillisIPAdress = 0;

JxWifiManager *wifi = new JxWifiManager();

// Eye Display (GC9A01 TFT) Initialization
// Pins: DC on GPIO0 (D3), CS on respective GPIO, RST set to 0.
Arduino_DataBus *leftBus = new Arduino_HWSPI(0 /* DC (D3) */, 16 /* CS (D0) */);
Arduino_GFX *leftEye = new Arduino_GC9A01(leftBus, 0 /* RST */);

Arduino_DataBus *rightBus = new Arduino_HWSPI(0 /* DC (D3) */, 15 /* CS (D8) */);
Arduino_GFX *rightEye = new Arduino_GC9A01(rightBus, 0 /* RST */);

// PWM Servo Driver (PCA9685) instance
Adafruit_PWMServoDriver *pwm = new Adafruit_PWMServoDriver(0x40);

// Huyang Robot Subsystem Instances
HuyangFace *huyangFace = new HuyangFace(leftEye, rightEye); // Manages eye animations
HuyangBody *huyangBody = new HuyangBody(pwm); // Manages body servos and chest lights
HuyangNeck *huyangNeck = new HuyangNeck(pwm); // Manages neck servos
HuyangAudio *huyangAudio = new HuyangAudio(); // Audio system

// Web Server instance, using the port defined in config.h
WebServer *webserver = new WebServer(WebServerPort);

// Include the main system logic (setup and loop functions)
// These functions will now use the globally defined objects.
#include "src/system.h"
