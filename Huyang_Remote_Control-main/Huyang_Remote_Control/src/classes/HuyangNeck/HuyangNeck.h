#ifndef HuyangNeck_h
#define HuyangNeck_h

#include "Arduino.h"
#include <Adafruit_PWMServoDriver.h>
#include "../EasingServo.h"
// Corrected include path for WebServer.h
#include "../WebServer/WebServer.h" // Include WebServer.h to access calibration data

// Servo Parameters
#define HuyangNeck_SERVOMIN 150	 // This is the 'minimum' pulse length count (out of 4096)
#define HuyangNeck_SERVOMAX 595	 // This is the 'maximum' pulse length count (out of 4096)
#define HuyangNeck_SERVO_FREQ 60 // Analog servos run at ~50 Hz updates

#define pwm_pin_head_monocle (uint8_t)4
#define pwm_pin_head_left (uint8_t)5
#define pwm_pin_head_right (uint8_t)6

#define pwm_pin_head_rotate (uint8_t)8
#define pwm_pin_head_neck (uint8_t)9

class HuyangNeck
{
public:
	// Modified constructor to accept a pointer to the WebServer object
	HuyangNeck(Adafruit_PWMServoDriver *pwm, WebServer *webserver);
	void setup();
	void loop();
	// Modified functions to accept optional calibration offset
	void tiltNeckSideways(double degree, int16_t calibrationOffset = 0);
	void tiltNeckForward(double degree, double duration = 1000, int16_t calibrationOffset = 0);
	void rotateHead(double degree, double duration = 1000, int16_t calibrationOffset = 0);
	
	// New: Function to set the monocle position
	void setMonoclePosition(int16_t position, int16_t calibrationOffset = 0);

	bool automatic = true;

	double targetTiltSideways = 0;
	double targetTiltForward = 0;
	double targetRotate = 0;
	double targetMonoclePosition = 0; // New: Target for monocle position

private:
	Adafruit_PWMServoDriver *_pwm;
	WebServer *_webserver; // New: Pointer to the WebServer object to access calibration data

	unsigned long _currentMillis = 0;
	unsigned long _previousMillis = 0;

	unsigned long _randomDoRotate = 0;
	unsigned long _randomDoTiltForward = 0;
	unsigned long _randomDoMonocle = 0; // New: for random monocle movement

	double _currentTiltSideways = 0;
	double _minTiltSideways = -100;
	double _maxTiltSideways = 100;
	double _startTiltSideways = 0;
	double _tiltSidewaysPercentage = 1.0;

	double _currentTiltForward = 50;
	double _minTiltForward = 0;
	double _maxTiltForward = 200;
	double _startTiltForward = 50;
	double _tiltForwardDuration = 0;
	unsigned long _tiltForwardStartMillis = 0;

	double _currentRotate = 0;
	double _minRotation = -100;
	double _maxRotation = 100;
	double _startRotate = 0;
	double _rotationDuration = 0;
	unsigned long _rotationStartMillis = 0;

	double _currentMonoclePosition = 0; // New: Current monocle position
	double _monocleDuration = 0;        // New: Monocle easing duration
	unsigned long _monocleStartMillis = 0; // New: Monocle easing start time
    double _startMonoclePosition = 0; // Explicitly declared here

	void rotateServo(uint8_t servo, double degree);
	void updateNeckPosition();
	void updateCurrentRotate();
	void updateMonoclePosition(); // New: Update for monocle easing
	double easeInOutQuad(double t);
	double easeInAndOut(double start, double current, double target, double percentage);
	int16_t getMaxSpeedRange(int16_t speed, int16_t step);

	void doRandomRotate();
	void doRandomTiltForward();
	void doRandomMonocle(); // New: For random monocle movement
};

#endif
