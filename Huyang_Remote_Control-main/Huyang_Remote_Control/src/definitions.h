#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// These are global variables and object instantiations used throughout the Huyang Droid project.
// They are declared as pointers to allow dynamic creation and to manage their lifetimes.

unsigned long currentMillis = 0;        // Stores the current time in milliseconds, updated every loop
unsigned long previousMillisIPAdress = 0; // Stores the last time IP address was printed

// --- WiFi Manager ---
// Manages the ESP's WiFi connection, either as an access point or client.
JxWifiManager *wifi = new JxWifiManager();

// --- Eye Display Drivers ---
// These objects manage the two TFT displays used as Huyang's eyes.
// Arduino_HWSPI sets up the hardware SPI bus for communication.
// Arduino_GC9A01 initializes the specific display controller.
Arduino_DataBus *leftBus = new Arduino_HWSPI(2 /* DC */, 16 /* CS */);
Arduino_GFX *leftEye = new Arduino_GC9A01(leftBus, 0 /* RST */);

Arduino_DataBus *rightBus = new Arduino_HWSPI(2 /* DC */, 15 /* CS */);
Arduino_GFX *rightEye = new Arduino_GC9A01(rightBus, 0 /* RST */);

// --- PWM Servo Driver ---
// This object controls the PCA9685 PWM driver board, which in turn controls the servos.
Adafruit_PWMServoDriver *pwm = new Adafruit_PWMServoDriver(0x40); // Default I2C address for PCA9685

// --- Web Server ---
// Manages the web interface for controlling the droid.
// THIS MUST BE DECLARED BEFORE COMPONENTS THAT RELY ON ITS POINTER.
WebServer *webserver = new WebServer(WebServerPort); // MOVED: Now declared before HuyangBody/HuyangNeck

// --- Huyang Droid Component Classes ---
// These are custom classes that encapsulate the logic for different parts of the droid.
// Their constructors now require the 'webserver' pointer to access shared state and calibration data.
HuyangFace *huyangFace = new HuyangFace(leftEye, rightEye); // HuyangFace constructor does not currently take webserver
HuyangBody *huyangBody = new HuyangBody(pwm, webserver); // MODIFIED: Pass webserver pointer
HuyangNeck *huyangNeck = new HuyangNeck(pwm, webserver); // MODIFIED: Pass webserver pointer

// --- Audio Manager ---
// Manages sound output for the droid. (Currently commented out in system.h setup/loop)
HuyangAudio *huyangAudio = new HuyangAudio();


#endif
