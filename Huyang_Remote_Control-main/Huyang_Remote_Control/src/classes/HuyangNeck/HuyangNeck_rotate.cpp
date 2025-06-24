#include "HuyangNeck.h"

// Modified rotateHead to accept an optional calibrationOffset
void HuyangNeck::rotateHead(double degree, double duration, int16_t calibrationOffset)
{
	// Apply the calibration offset to the incoming degree
	double calibratedDegree = degree + calibrationOffset;

	// Constrain the calibrated degree to the defined min/max rotation range
	calibratedDegree = constrain(calibratedDegree, _minRotation, _maxRotation);

	// If the new target is different from the current one, update the target
	// and calculate the duration for the movement.
	if (targetRotate != calibratedDegree)
	{
		// If duration is 0, calculate a default duration based on the distance to move.
		// This provides a smooth, speed-proportional movement if no explicit duration is given.
		if (duration == 0)
		{
			double way = 0;
			if (calibratedDegree > _currentRotate)
			{
				way = calibratedDegree - _currentRotate;
			}
			else
			{
				way = _currentRotate - calibratedDegree;
			}

			duration = way * 16; // 16ms per unit of 'way' for speed control
		}

		_startRotate = _currentRotate; // Store the starting position for easing
		targetRotate = calibratedDegree;
		_rotationDuration = duration;
		_rotationStartMillis = _currentMillis; // Record the start time for easing calculation
	}
}

void HuyangNeck::updateCurrentRotate()
{
	if (_currentRotate != targetRotate)
	{
		double percentage = (_currentMillis - _rotationStartMillis) / _rotationDuration;
		_currentRotate = easeInAndOut(_startRotate, _currentRotate, targetRotate, percentage);

		// Apply the stored neck rotation calibration offset before mapping to servo degrees.
		// The _webserver pointer is now available for accessing calNeckRotation.
		// The calibration value is added directly as it's an offset to the neutral position.
		double finalRotateDegree = _currentRotate + _webserver->calNeckRotation; // Declared 'finalRotateDegree' with type

		// Map the calibrated rotation value to actual servo degrees (0-110).
		// These ranges are specific to the servo and might need fine-tuning.
		uint16_t rotateDegree = map(finalRotateDegree, _minRotation, _maxRotation, 0, 110);
		rotateServo(pwm_pin_head_rotate, rotateDegree);
	}
}
