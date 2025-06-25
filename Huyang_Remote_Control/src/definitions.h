// This file contains global variable and object *declarations* for the Huyang Droid's subsystems.
// Their *definitions* (memory allocations) are handled in Huyang_Remote_Control.ino.

#include <Arduino.h>
#include <Arduino_GFX_Library.h>      // For TFT displays (eyes)
#include <Adafruit_PWMServoDriver.h>  // For servo motor control
#include "submodules/JxWifiManager/JxWifiManager.h" // For Wi-Fi management
#include "classes/HuyangFace/HuyangFace.h"        // For controlling the robot's face/eyes
#include "classes/HuyangBody/HuyangBody.h"        // For controlling the robot's body movements and lights
#include "classes/HuyangNeck/HuyangNeck.h"        // For controlling the robot's neck movements
#include "classes/HuyangAudio/HuyangAudio.h"      // For audio playback
#include "classes/WebServer/WebServer.h"          // For the web interface

// Global variables for time tracking (extern declarations)
extern unsigned long currentMillis;
extern unsigned long previousMillisIPAdress;

// Wi-Fi Manager instance (extern declaration)
extern JxWifiManager *wifi;

// --- Eye Display (GC9A01 TFT) Instances (extern declarations) ---
extern Arduino_DataBus *leftBus;
extern Arduino_GFX *leftEye;
extern Arduino_DataBus *rightBus;
extern Arduino_GFX *rightEye;

// PWM Servo Driver (PCA9685) instance (extern declaration)
extern Adafruit_PWMServoDriver *pwm;

// Huyang Robot Subsystem Instances (extern declarations)
extern HuyangFace *huyangFace;
extern HuyangBody *huyangBody;
extern HuyangNeck *huyangNeck;
extern HuyangAudio *huyangAudio;

// Web Server instance (extern declaration)
extern WebServer *webserver;
