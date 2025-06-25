#ifndef HuyangBody_h
#define HuyangBody_h

#include "Arduino.h"
#include <Adafruit_PWMServoDriver.h> // For servo motor control
#include <Adafruit_NeoPixel.h>       // For NeoPixel (chest lights) control

// Servo Parameters for PCA9685 PWM Driver
#define HuyangBody_SERVOMIN 150	 // This is the 'minimum' pulse length count (out of 4096)
#define HuyangBody_SERVOMAX 595	 // This is the 'maximum' pulse length count (out of 4096)
#define HuyangBody_SERVO_FREQ 60 // Analog servos typically run at ~50 Hz updates

// PWM channel pins for body servos on the PCA9685 board
#define pwm_pin_sideway_left (uint8_t)14  // Left body sideways tilt servo
#define pwm_pin_sideway_right (uint8_t)15 // Right body sideways tilt servo
#define pwm_pin_forward_left (uint8_t)12  // Left body forward tilt servo
#define pwm_pin_forward_right (uint8_t)13 // Right body forward tilt servo
#define pwm_pin_body_rotate (uint8_t)11   // Body rotation servo

// NeoPixel pin and format
#define NEO_PIXEL_PIN (uint8_t)0    // Pin connected to NeoPixels (can be any GPIO, confirm config)
#define NEO_PIXEL_COUNT 2           // Number of NeoPixels (2 LEDs for chest lights)
#define pixelFormat NEO_GRB + NEO_KHZ800 // Pixel color order and frequency

class HuyangBody
{
public:
	// Constructor: takes a pointer to the PWM driver
	HuyangBody(Adafruit_PWMServoDriver *pwm);

	// Setup function: initializes NeoPixels and performs initial servo centering
	void setup();
	// Loop function: called repeatedly to update servo positions and handle light animations
	void loop();

	// Flag to enable/disable automatic animations
	bool automatic = true;

	// Public control functions for body movements
	void tiltBodySideways(int16_t degree);
	void tiltBodyForward(int16_t degree);
	void rotateBody(int16_t degree);

	// Sets all body servos to their center positions
	void centerAll();

	// --- NEW: Calibration offsets ---
	// These values will be added/subtracted to the target degrees
	int16_t calibrationRotate = 0;
	int16_t calibrationTiltForward = 0;
	int16_t calibrationTiltSideways = 0;

	// --- NEW: Chest Light Control ---
	enum LightMode
	{
		LIGHT_OFF = 0,
		LIGHT_ON = 1,
		LIGHT_BLINK_ALTERNATE = 2
	};
	LightMode currentLightMode = LIGHT_ON; // Current operating mode for chest lights
	void updateChestLights(); // Function to manage chest light behavior

private:
	Adafruit_PWMServoDriver *_pwm;      // Pointer to the PWM driver instance
	Adafruit_NeoPixel *_neoPixelLights; // Pointer to the NeoPixel object

	unsigned long _currentMillis = 0;   // Current time in milliseconds
	unsigned long _previousMillis = 0;  // Previous time for general timing

	// Timers for automatic body movements
	unsigned long _randomDoRotate = 0;
	unsigned long _randomDoTiltForward = 0;
	unsigned long _randomDoTiltSideways = 0;

	// Timers for chest light animations
	unsigned long _lastLightToggleMillis = 0;
	uint16_t _blinkInterval = 500; // Milliseconds for blink interval

	// Private helper methods
	// Maps a degree value to a PWM pulselength and sends it to the specified servo pin
	void rotateServo(uint8_t servo, uint16_t degree);

	// Internal update functions for easing (if applicable, current body movements are direct)
	// Add similar easing variables and functions here if smoothing is desired for body movements.

	// Functions for generating random movements
	void doRandomRotate();
	void doRandomTiltForward();
	void doRandomTiltSideways();

	// Internal light control functions
	void setAllLights(uint32_t color);
	void setLight(uint8_t pixelNum, uint32_t color);
};

#endif
