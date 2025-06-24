#include "HuyangNeck.h"
#include <Adafruit_PWMServoDriver.h>

// Constructor: Initializes the HuyangNeck with a PWM driver and a WebServer pointer.
// The WebServer pointer is used to access calibration data.
HuyangNeck::HuyangNeck(Adafruit_PWMServoDriver *pwm, WebServer *webserver)
{
	_pwm = pwm;
	_webserver = webserver; // Store the WebServer pointer
}

// Helper function to convert a degree value to a pulselength for the PWM servo driver.
// This uses the SERVOMIN/SERVOMAX defined in HuyangNeck.h to map 0-180 degrees to PWM values.
void HuyangNeck::rotateServo(uint8_t servo, double degree)
{
	uint16_t pulselength = map(degree, 0, 180, HuyangNeck_SERVOMIN, HuyangNeck_SERVOMAX);
	_pwm->setPWM(servo, 0, pulselength);
}

// Easing function: Quadratic ease-in-out for smooth acceleration and deceleration.
// 't' is the normalized time (0.0 to 1.0).
double HuyangNeck::easeInOutQuad(double t)
{
	return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
}

// General easing function for moving a value smoothly between a start and target.
// It uses easeInOutQuad for the animation curve.
double HuyangNeck::easeInAndOut(double start, double current, double target, double percentage)
{
	double result = target; // Default to target if percentage is 1.0 or more

	if (percentage > 1.0)
	{
		percentage = 1.0; // Cap percentage at 1.0
	}

	if (current != target)
	{
		// Calculate the eased value based on the percentage
		double easeInOut = easeInOutQuad(percentage);

		if (current < target)
		{
			double subTarget = (target - start);
			result = start + (subTarget * easeInOut);
			// Ensure we don't overshoot the target
			if (result > target)
			{
				result = target;
			}
		}
		else if (current > target)
		{
			double subTarget = (start - target);
			result = start - (subTarget * easeInOut);
			// Ensure we don't undershoot the target
			if (result < target)
			{
				result = target;
			}
		}
	}
	return result;
}

// Calculates a speed range for random movements.
// Currently not used in new random functions, but kept for potential future use.
int16_t HuyangNeck::getMaxSpeedRange(int16_t speed, int16_t step)
{
	int16_t result = speed;
	if (speed > 0)
	{
		result = result + random(0, step);
	}
	else
	{
		result = result - random(0, step);
	}
	return result;
}

void HuyangNeck::setup()
{
	// Initialization logic for the neck servos, if any specific setup is needed.
	// Servos are typically controlled via PWM commands in loop().
}

void HuyangNeck::loop()
{
	_currentMillis = millis(); // Update current time

	// Handle millis() rollover
	if (_previousMillis > _currentMillis)
	{
		_previousMillis = _currentMillis;
	}

    // Check the servosLockedForCalibration flag from the WebServer.
    // If true, the main system loop (in system.h) will prevent calling
    // the manual control functions (rotateHead, tiltNeckForward, tiltNeckSideways, setMonoclePosition).
    // In this state, the neck components should ideally hold their current position or a safe middle.
    // Their `update` functions will continue to run to smoothly move towards the target set
    // when 'setMiddleAndLock' was called, or simply hold the current position if no movement is commanded.

	// If automatic mode is enabled, trigger random movements
	if (automatic == true)
	{
		doRandomRotate();
		doRandomTiltForward();
        doRandomMonocle(); // NEW: Call random monocle movement
	}

	// Update servo positions based on current target and easing calculations
	updateNeckPosition(); // Handles tilt forward/sideways
	updateCurrentRotate(); // Handles head rotation
    updateMonoclePosition(); // NEW: Handles monocle position updates
}

// Sets the target position for the monocle and initiates easing.
// 'position' is an arbitrary value (e.g., 0-100) representing the monocle's range.
// 'duration' is the time in milliseconds for the movement to complete.
void HuyangNeck::setMonoclePosition(int16_t position, int16_t duration)
{
    // Constrain the input position to a valid range for the monocle (e.g., 0 to 100)
    // These min/max values are based on the expected range of monocle servo movement.
    position = constrain(position, 0, 100);

    // Only update if the target position is different from the current target
    if (targetMonoclePosition != position)
    {
        // If duration is 0, calculate a default duration for snappy movement
        if (duration == 0)
        {
            double way = abs(position - _currentMonoclePosition);
            duration = way * 10; // Adjust multiplier for desired default speed
        }

        _startMonoclePosition = _currentMonoclePosition; // Store current position as start for easing
        targetMonoclePosition = position; // Set the new target position
        _monocleDuration = duration;      // Set the duration for the easing
        _monocleStartMillis = _currentMillis; // Record the start time for easing calculation
    }
}

// Updates the current monocle position based on easing and applies it to the servo.
void HuyangNeck::updateMonoclePosition()
{
    // Only update if the current position hasn't reached the target
    if (_currentMonoclePosition != targetMonoclePosition)
    {
        double percentage = (_currentMillis - _monocleStartMillis) / _monocleDuration;
        _currentMonoclePosition = easeInAndOut(_startMonoclePosition, _currentMonoclePosition, targetMonoclePosition, percentage);

        // Apply the stored monocle calibration offset.
        // The monocle calibration offset is accessed via the _webserver pointer.
        double finalMonocleDegree = _currentMonoclePosition + _webserver->calMonoclePosition;

        // Map the monocle position (e.g., 0-100) to actual servo degrees (0-180).
        // These servo degrees will need calibration for the specific monocle servo and setup.
        // Assuming 0 position is one end of its travel, and 180 is the other.
        uint16_t servoDegree = map(finalMonocleDegree, 0, 100, 0, 180);
        rotateServo(pwm_pin_head_monocle, servoDegree); // Apply the rotation to the monocle servo pin
    }
}


void HuyangNeck::updateNeckPosition()
{
	// Update tilt forward position using easing
	if (_currentTiltForward != targetTiltForward)
	{
		double tiltForwardPercentage = (_currentMillis - _tiltForwardStartMillis) / _tiltForwardDuration;
		_currentTiltForward = easeInAndOut(_startTiltForward, _currentTiltForward, targetTiltForward, tiltForwardPercentage);
	}

	// Update tilt sideways position using easing (percentage-based)
	if (_tiltSidewaysPercentage < 1.0)
	{
		_currentTiltSideways = easeInAndOut(_startTiltSideways, _currentTiltSideways, targetTiltSideways, _tiltSidewaysPercentage);
		_tiltSidewaysPercentage += 0.06; // Increment percentage for gradual movement
	}

	// Calculate and apply calibration offsets before mapping to servo degrees
	double calibratedTiltForward = _currentTiltForward + _webserver->calNeckTiltForward;
	double calibratedTiltSideways = _currentTiltSideways + _webserver->calNeckTiltSideways;

	// Calculate individual servo degrees based on combined forward and sideways tilt
	// These mappings are highly dependent on the physical servo setup.
	uint16_t leftDegree = map(calibratedTiltForward + calibratedTiltSideways, 0, 200, 65, 10);
	uint16_t rightDegree = map(calibratedTiltForward - calibratedTiltSideways, 0, 200, 35, 90);
	uint16_t neckDegree = map(calibratedTiltForward, 0, 200, 100, 0); // Assuming a central neck servo

	// Constrain degrees to ensure they are within safe operating limits for the servos
	leftDegree = constrain(leftDegree, (uint16_t)10, (uint16_t)65);
	rightDegree = constrain(rightDegree, (uint16_t)35, (uint16_t)90);
	neckDegree = constrain(neckDegree, (uint16_t)0, (uint16_t)100);

	// Apply rotations to the respective servo pins
	rotateServo(pwm_pin_head_left, leftDegree);
	rotateServo(pwm_pin_head_right, rightDegree);
	rotateServo(pwm_pin_head_neck, neckDegree);
}
