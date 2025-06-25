#ifndef HuyangNeck_h
#define HuyangNeck_h

#include "Arduino.h"
#include <Adafruit_PWMServoDriver.h>
#include "../EasingServo.h" // Include EasingServo for smooth movements

// Servo Parameters for PCA9685 PWM Driver
#define HuyangNeck_SERVOMIN 150	 // This is the 'minimum' pulse length count (out of 4096)
#define HuyangNeck_SERVOMAX 595	 // This is the 'maximum' pulse length count (out of 4096)
#define HuyangNeck_SERVO_FREQ 60 // Analog servos typically run at ~50 Hz updates, 60 is fine for PCA9685

// PWM channel pins for neck servos on the PCA9685 board
#define pwm_pin_head_monocle (uint8_t)4 // Servo for monacle movement
#define pwm_pin_head_left (uint8_t)5    // Left neck servo for tilt
#define pwm_pin_head_right (uint8_t)6   // Right neck servo for tilt
#define pwm_pin_head_rotate (uint8_t)8  // Head rotation servo
#define pwm_pin_head_neck (uint8_t)9    // Main neck servo for forward/backward tilt

class HuyangNeck
{
public:
	// Constructor: takes a pointer to the PWM driver
	HuyangNeck(Adafruit_PWMServoDriver *pwm);

	// Setup function: performs initial servo centering or setup
	void setup();
	// Loop function: called repeatedly to update servo positions and handle automatic animations
	void loop();

	// Public control functions for neck movements
	void tiltNeckSideways(double degree);
	void tiltNeckForward(double degree, double duration = 1000);
	void rotateHead(double degree, double duration = 1000);

	// Flag to enable/disable automatic (random) animations
	bool automatic = true;

	// Target degrees for manual control (set by web server)
	double targetTiltSideways = 0;
	double targetTiltForward = 0;
	double targetRotate = 0;

	// --- NEW: Calibration offsets ---
	// These values will be added/subtracted to the target degrees
	// before mapping to pulse lengths, effectively shifting the servo's range.
	int16_t calibrationRotate = 0;
	int16_t calibrationTiltForward = 0;
	int16_t calibrationTiltSideways = 0;


private:
	Adafruit_PWMServoDriver *_pwm; // Pointer to the PWM driver instance

	unsigned long _currentMillis = 0;  // Current time in milliseconds
	unsigned long _previousMillis = 0; // Previous time for general timing
	
	// Timers for random automatic movements
	unsigned long _randomDoRotate = 0;
	unsigned long _randomDoTiltForward = 0;
	unsigned long _randomDoTiltSideways = 0; // NEW: For random sideways tilt

	// Internal state variables for smooth movement easing
	double _currentTiltSideways = 0;
	double _minTiltSideways = -100; // Min value for sideways tilt input
	double _maxTiltSideways = 100;  // Max value for sideways tilt input
	double _startTiltSideways = 0;  // Starting position for current sideways tilt movement
	double _tiltSidewaysPercentage = 1.0; // Easing percentage for sideways tilt

	double _currentTiltForward = 50; // Current forward tilt position
	double _minTiltForward = 0;      // Min value for forward tilt input
	double _maxTiltForward = 200;    // Max value for forward tilt input
	double _startTiltForward = 50;   // Starting position for current forward tilt movement
	double _tiltForwardDuration = 0; // Duration of forward tilt movement
	unsigned long _tiltForwardStartMillis = 0; // Start time of forward tilt movement

	double _currentRotate = 0;   // Current rotation position
	double _minRotation = -100;  // Min value for rotation input
	double _maxRotation = 100;   // Max value for rotation input
	double _startRotate = 0;     // Starting position for current rotation movement
	double _rotationDuration = 0; // Duration of rotation movement
	unsigned long _rotationStartMillis = 0; // Start time of rotation movement

	// Private helper methods
	// Maps a degree value to a PWM pulselength and sends it to the specified servo pin
	void rotateServo(uint8_t servo, double degree);
	// Easing function for smooth animation (quadrilateral ease-in-out)
	double easeInOutQuad(double t);
	// Generic easing function to calculate intermediate position
	double easeInAndOut(double start, double current, double target, double percentage);

	// Update functions for each type of neck movement, applying easing and calibration
	void updateNeckPosition();
	void updateCurrentRotate();
	void doRandomRotate();
	void doRandomTiltForward();
	void doRandomTiltSideways(); // NEW: For random sideways tilt
};

#endif
