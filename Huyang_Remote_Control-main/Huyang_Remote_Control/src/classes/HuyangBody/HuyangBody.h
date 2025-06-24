#ifndef HuyangBody_h
#define HuyangBody_h

#include "Arduino.h"
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
// Corrected include path for WebServer.h
#include "../WebServer/WebServer.h" // Include WebServer.h to access calibration and LightMode

// Servo Parameters
#define HuyangBody_SERVOMIN 150	 // This is the 'minimum' pulse length count (out of 4096)
#define HuyangBody_SERVOMAX 595	 // This is the 'maximum' pulse length count (out of 4096)
#define HuyangBody_SERVO_FREQ 60 // Analog servos run at ~50 Hz updates

#define pwm_pin_sideway_left (uint8_t)14
#define pwm_pin_sideway_right (uint8_t)15
#define pwm_pin_forward_left (uint8_t)12
#define pwm_pin_forward_right (uint8_t)13
#define pwm_pin_body_rotate (uint8_t)11

class HuyangBody
{
public:
	// Modified constructor to accept a pointer to the WebServer object
	HuyangBody(Adafruit_PWMServoDriver *pwm, WebServer *webserver);
	void setup();
	void loop();

	bool automatic = true;

	// Modified functions to accept optional calibration offset
	void tiltBodySideways(int16_t degree, int16_t calibrationOffset = 0);
	void tiltBodyForward(int16_t degree, int16_t calibrationOffset = 0);
	void rotateBody(int16_t degree, int16_t calibrationOffset = 0);

	void centerAll(); // This function will also need to consider calibration offsets when called

	// New: Function to set the torso light mode
	void setTorsoLightMode(uint16_t mode);

private:
	Adafruit_PWMServoDriver *_pwm;
	WebServer *_webserver; // New: Pointer to the WebServer object to access calibration and light mode

	unsigned long _currentMillis = 0;
	unsigned long _previousMillis = 0;

	uint16_t _currentLightMode = WebServer::LightModeOff; // Current light mode
	unsigned long _lastLightChangeMillis = 0;
	uint16_t _lightBlinkInterval = 0; // For blinking patterns
	bool _lightState = false; // For blinking patterns

	void rotateServo(uint8_t servo, uint16_t degree);

	Adafruit_NeoPixel *_neoPixelLights;
	neoPixelType pixelFormat = NEO_GRB + NEO_KHZ800; // From original file

	// Helper functions for light patterns
	void updatePoliceLights();
	void updateDiscoLights();
	void updateBlinkLights();
};

#endif
