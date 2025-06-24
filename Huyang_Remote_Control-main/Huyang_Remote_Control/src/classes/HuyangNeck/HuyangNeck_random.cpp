#include "HuyangNeck.h"

void HuyangNeck::doRandomRotate()
{
	if (_randomDoRotate == 0)
	{
		_randomDoRotate = _currentMillis + 2000 + (random(6, 12 + 1) * 1000);
	}

	if (_currentMillis > _randomDoRotate)
	{
		_randomDoRotate = 0;

		if (_currentRotate > 0)
		{
			rotateHead(-(random(10, 80 + 1)), random(2, 6 + 1) * 1000);
		}
		else
		{
			rotateHead(random(10, 80 + 1), random(2, 6 + 1) * 1000);
		}
	}
}

void HuyangNeck::doRandomTiltForward()
{
	if (_randomDoTiltForward == 0)
	{
		_randomDoTiltForward = _currentMillis + 2500 + (random(6, 12 + 1) * 1050);
	}

	if (_currentMillis > _randomDoTiltForward)
	{
		_randomDoTiltForward = 0;

		double workArea = 60;
		double center = -15;

		tiltNeckForward(random(center - workArea, center + workArea + 1), random(3, 6 + 1) * 1000);
	}
}

// New: Implementation for doRandomMonocle
void HuyangNeck::doRandomMonocle()
{
    if (_randomDoMonocle == 0)
    {
        // Set next random trigger time: wait 5-10 seconds before next monocle action
        _randomDoMonocle = _currentMillis + 5000 + (random(0, 5) * 1000);
    }

    if (_currentMillis > _randomDoMonocle)
    {
        _randomDoMonocle = 0; // Reset timer for next cycle

        // Randomly decide if the monocle should move to a "visible" or "hidden" position
        // Assuming 0 is hidden, and 100 is fully extended/visible
        int16_t targetPosition;
        if (random(0, 2) == 0) { // 50% chance to go to a visible state
            targetPosition = random(50, 101); // Random position between 50 and 100 (visible range)
        } else {
            targetPosition = random(0, 51); // Random position between 0 and 50 (mostly hidden)
        }

        // Call the setMonoclePosition with a random duration (e.g., 500ms to 2000ms)
        setMonoclePosition(targetPosition, random(500, 2001));
    }
}
