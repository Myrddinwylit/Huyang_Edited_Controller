#include "HuyangNeck.h"
#include <Adafruit_PWMServoDriver.h>

HuyangNeck::HuyangNeck(Adafruit_PWMServoDriver *pwm)
{
	_pwm = pwm;
}

void HuyangNeck::setup()
{
	// Initial centering or setup for servos if needed
	// Neck servos will be centered based on their default position and calibration in loop()
}

// Maps a degree value to a PWM pulselength and sends it to the specified servo pin
void HuyangNeck::rotateServo(uint8_t servo, double degree)
{
	// Map the input degree (0-180 for standard servos) to the PCA9685 pulse length range
	uint16_t pulselength = map(degree, 0, 180, HuyangNeck_SERVOMIN, HuyangNeck_SERVOMAX);

	// Set the PWM output for the specified servo pin
	_pwm->setPWM(servo, 0, pulselength);
}

// Easing function: Ease-in-out quadratic
double HuyangNeck::easeInOutQuad(double t)
{
	return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
}

// Generic easing function to calculate intermediate position
double HuyangNeck::easeInAndOut(double start, double current, double target, double percentage)
{
	double result = target; // Default to target if percentage is >= 1.0

	if (percentage > 1.0)
	{
		percentage = 1.0; // Cap percentage at 1.0 (100%)
	}

	if (current != target) // Only calculate if movement is needed
	{
		double easeInOut = easeInOutQuad(percentage); // Get eased percentage

		if (current < target)
		{
			double subTarget = (target - start);
			result = start + (subTarget * easeInOut);
			if (result > target) { // Ensure we don't overshoot
				result = target;
			}
		}
		else if (current > target)
		{
			double subTarget = (start - target);
			result = start - (subTarget * easeInOut);
			if (result < target) { // Ensure we don't overshoot
				result = target;
			}
		}
	}
	return result;
}

// Main loop for HuyangNeck, called repeatedly from system.h
void HuyangNeck::loop()
{
	_currentMillis = millis(); // Get current time

	// Reset previousMillis if system time rolls over
	if (_previousMillis > _currentMillis)
	{
		_previousMillis = _currentMillis;
	}

	// Update servo positions based on target and easing
	updateCurrentRotate();
	updateNeckPosition(); // Updates tilt forward and sideways

	// If in automatic mode, trigger random movements
	if (automatic == true)
	{
		doRandomRotate();
		doRandomTiltForward();
		doRandomTiltSideways(); // Call random sideways tilt
	}
}

// --- Neck Movement Control Functions ---

// Controls head rotation
void HuyangNeck::rotateHead(double degree, double duration)
{
	// Apply calibration offset
	degree += calibrationRotate; // Use the member variable from HuyangNeck class
	// Constrain degree within allowed range
	degree = constrain(degree, _minRotation, _maxRotation);

	if (targetRotate != degree) // Only update if target has changed
	{
		if (duration == 0) // Calculate duration if not provided (based on distance)
		{
			double way = abs(degree - _currentRotate); // Distance to move
			duration = way * 16; // Adjust this multiplier for desired speed
		}

		_startRotate = _currentRotate; // Store starting point for easing
		targetRotate = degree;         // Set new target
		_rotationDuration = duration;  // Set movement duration
		_rotationStartMillis = _currentMillis; // Record start time of movement
	}
}

// Controls neck tilt forward/backward
void HuyangNeck::tiltNeckForward(double degree, double duration)
{
	// Apply calibration offset
	degree += calibrationTiltForward; // Use the member variable from HuyangNeck class
	// Adjust degree for internal mapping, then constrain
	degree = degree + 100; // Shifts -100 to 100 range to 0 to 200 for mapping
	degree = constrain(degree, _minTiltForward, _maxTiltForward);

	if (targetTiltForward != degree) // Only update if target has changed
	{
		if (duration == 0) // Calculate duration if not provided
		{
			double way = abs(degree - _currentTiltForward);
			duration = way * 16;
		}

		_startTiltForward = _currentTiltForward;
		targetTiltForward = degree;
		_tiltForwardDuration = duration;
		_tiltForwardStartMillis = _currentMillis;
	}
}

// Controls neck tilt sideways
void HuyangNeck::tiltNeckSideways(double degree)
{
	// Apply calibration offset
	degree += calibrationTiltSideways; // Use the member variable from HuyangNeck class
	// Constrain degree within allowed range
	degree = constrain(degree, _minTiltSideways, _maxTiltSideways);

	// Map input range (-100 to 100) to a more internal working range (-50 to 50)
	// This internal range is then used for calculating left/right servo positions.
	degree = map(degree, _minTiltSideways, _maxTiltSideways, -50, 50);

	if (targetTiltSideways != degree) // Only update if target has changed
	{
		_startTiltSideways = _currentTiltSideways; // Store starting point for easing
		targetTiltSideways = degree;              // Set new target
		_tiltSidewaysPercentage = 0.0;            // Reset easing percentage
	}
}


// --- Internal Update Functions for Easing ---

// Updates the current rotation position using easing
void HuyangNeck::updateCurrentRotate()
{
	if (_currentRotate != targetRotate) // If still moving towards target
	{
		// Calculate easing percentage based on time elapsed
		double percentage = (double)(_currentMillis - _rotationStartMillis) / _rotationDuration;
		// Update current position using easing function
		_currentRotate = easeInAndOut(_startRotate, _currentRotate, targetRotate, percentage);

		// Map the internal rotation value to the servo's actual degree range (0-110 for example)
		uint16_t rotateDegree = map(_currentRotate, _minRotation, _maxRotation, 0, 110);
		rotateServo(pwm_pin_head_rotate, rotateDegree); // Send command to servo
	}
}

// Updates the current neck tilt forward and sideways positions using easing
void HuyangNeck::updateNeckPosition()
{
	// Update forward tilt position
	if (_currentTiltForward != targetTiltForward)
	{
		double tiltForwardPercentage = (double)(_currentMillis - _tiltForwardStartMillis) / _tiltForwardDuration;
		_currentTiltForward = easeInAndOut(_startTiltForward, _currentTiltForward, targetTiltForward, tiltForwardPercentage);
	}

	// Update sideways tilt position (using a simple percentage increment for now)
	// This might need more sophisticated easing if a `duration` parameter is added to `tiltNeckSideways`
	if (_currentTiltSideways != targetTiltSideways)
	{
		_currentTiltSideways = easeInAndOut(_startTiltSideways, _currentTiltSideways, targetTiltSideways, _tiltSidewaysPercentage);
		_tiltSidewaysPercentage += 0.06; // Increment percentage for gradual movement
		if (_tiltSidewaysPercentage > 1.0) _tiltSidewaysPercentage = 1.0;
	}

	// Calculate individual servo degrees based on combined forward and sideways tilt
	// These mappings are crucial for coordinating multiple servos for complex movements.
	uint16_t leftDegree = map(_currentTiltForward + _currentTiltSideways, 0, 200, 65, 10);
	uint16_t rightDegree = map(_currentTiltForward - _currentTiltSideways, 0, 200, 35, 90);
	uint16_t neckDegree = map(_currentTiltForward, 0, 200, 100, 0);

	// Constrain final servo degrees to their physical limits
	leftDegree = constrain(leftDegree, (uint16_t)10, (uint16_t)65);
	rightDegree = constrain(rightDegree, (uint16_t)35, (uint16_t)90);
	neckDegree = constrain(neckDegree, (uint16_t)0, (uint16_t)100);

	// Send commands to respective servos
	rotateServo(pwm_pin_head_left, leftDegree);
	rotateServo(pwm_pin_head_right, rightDegree);
	rotateServo(pwm_pin_head_neck, neckDegree);
}

// --- Random Movement Functions (for automatic mode) ---

void HuyangNeck::doRandomRotate()
{
	if (_randomDoRotate == 0) // If no random rotation is scheduled
	{
		// Schedule next random rotation after an initial delay + random interval
		_randomDoRotate = _currentMillis + 2000 + (random(6, 12 + 1) * 1000);
	}

	if (_currentMillis > _randomDoRotate) // If scheduled time has passed
	{
		_randomDoRotate = 0; // Reset schedule

		// Randomly choose to rotate left or right
		if (_currentRotate > 0) // If currently rotated right, rotate left
		{
			rotateHead(-(random(10, 80 + 1)), random(2, 6 + 1) * 1000);
		}
		else // If currently rotated left or center, rotate right
		{
			rotateHead(random(10, 80 + 1), random(2, 6 + 1) * 1000);
		}
	}
}

void HuyangNeck::doRandomTiltForward()
{
	if (_randomDoTiltForward == 0) // If no random forward tilt is scheduled
	{
		// Schedule next random tilt after an initial delay + random interval
		_randomDoTiltForward = _currentMillis + 2500 + (random(6, 12 + 1) * 1050);
	}

	if (_currentMillis > _randomDoTiltForward) // If scheduled time has passed
	{
		_randomDoTiltForward = 0; // Reset schedule

		double workArea = 60;  // Range for random tilt
		double center = -15;   // Center point for tilt

		// Generate random tilt within a range around the center
		tiltNeckForward(random(center - workArea, center + workArea + 1), random(3, 6 + 1) * 1000);
	}
}

// Function for random sideways tilt (for automatic mode)
void HuyangNeck::doRandomTiltSideways()
{
	if (_randomDoTiltSideways == 0) // If no random sideways tilt is scheduled
	{
		// Schedule next random tilt after an initial delay + random interval
		_randomDoTiltSideways = _currentMillis + 3000 + (random(5, 10 + 1) * 1100);
	}

	if (_currentMillis > _randomDoTiltSideways) // If scheduled time has passed
	{
		_randomDoTiltSideways = 0; // Reset schedule

		double workArea = 30; // Range for random tilt
		double center = 0;    // Center point for tilt

		// Generate random tilt within a range around the center
		tiltNeckSideways(random(center - workArea, center + workArea + 1)); // Duration not specified for this one in webserver
	}
}
