#include "HuyangNeck.h"

// Modified tiltNeckSideways to accept an optional calibrationOffset
void HuyangNeck::tiltNeckSideways(double degree, int16_t calibrationOffset)
{
	// Apply the calibration offset to the incoming degree
	double calibratedDegree = degree + calibrationOffset;

	// Constrain the calibrated degree to the defined min/max tilt sideways range
	calibratedDegree = constrain(calibratedDegree, _minTiltSideways, _maxTiltSideways);

	// Map the calibrated degree from its internal range (-100 to 100) to a smaller,
	// more manageable range for the servo calculation (-50 to 50).
	// This mapping might need fine-tuning based on physical servo limits.
	calibratedDegree = map(calibratedDegree, _minTiltSideways, _maxTiltSideways, -50, 50);

	// If the new target is different from the current one, update the target
	// and reset the easing percentage to start a new movement.
	if (targetTiltSideways != calibratedDegree)
	{
		_startTiltSideways = _currentTiltSideways; // Store the starting position for easing
		targetTiltSideways = calibratedDegree;
		_tiltSidewaysPercentage = 0.0; // Reset easing progress
	}
}

// Modified tiltNeckForward to accept an optional calibrationOffset
void HuyangNeck::tiltNeckForward(double degree, double duration, int16_t calibrationOffset)
{
	// Apply the calibration offset to the incoming degree
	double calibratedDegree = degree + calibrationOffset;

	// Add 100 to shift the range from e.g., -100 to 100, to 0 to 200,
	// making it suitable for mapping to positive servo angles.
	calibratedDegree = calibratedDegree + 100;
	
	// Constrain the calibrated degree to the defined min/max tilt forward range
	calibratedDegree = constrain(calibratedDegree, _minTiltForward, _maxTiltForward);

	// If the new target is different from the current one, update the target
	// and calculate the duration for the movement.
	if (targetTiltForward != calibratedDegree)
	{
		// If duration is 0, calculate a default duration based on the distance to move.
		// This provides a smooth, speed-proportional movement if no explicit duration is given.
		if (duration == 0)
		{
			double way = 0;
			if (calibratedDegree > _currentTiltForward)
			{
				way = calibratedDegree - _currentTiltForward;
			}
			else
			{
				way = _currentTiltForward - calibratedDegree;
			}
			duration = way * 16; // 16ms per unit of 'way' for speed control
		}

		_startTiltForward = _currentTiltForward; // Store the starting position for easing
		targetTiltForward = calibratedDegree;
		_tiltForwardDuration = duration;
		_tiltForwardStartMillis = _currentMillis; // Record the start time for easing calculation
	}
}
