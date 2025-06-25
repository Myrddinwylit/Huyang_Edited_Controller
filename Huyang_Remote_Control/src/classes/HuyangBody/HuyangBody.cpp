#include "HuyangBody.h"
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>

// Corrected typo: Adafruit_PWMServoDriver
HuyangBody::HuyangBody(Adafruit_PWMServoDriver *pwm)
{
	_pwm = pwm;
	// Initialize NeoPixel object for 2 pixels on NEO_PIXEL_PIN
	// This pin MUST be defined in config.h or similar if it's not a fixed value.
	_neoPixelLights = new Adafruit_NeoPixel(NEO_PIXEL_COUNT, NEO_PIXEL_PIN, pixelFormat);
	_neoPixelLights->setBrightness(20); // Set initial brightness (0-255)
	_neoPixelLights->begin(); // Initialize NeoPixel library
}

void HuyangBody::setup()
{
	// Initial centering of all body servos
	centerAll();
	_neoPixelLights->show(); // Ensure NeoPixels are off or at their default state
	updateChestLights(); // Set initial light mode
}

// Maps a degree value to a PWM pulselength and sends it to the specified servo pin
void HuyangBody::rotateServo(uint8_t servo, uint16_t degree)
{
	// Map the input degree (0-180 for standard servos) to the PCA9685 pulse length range
	uint16_t pulselength = map(degree, 0, 180, HuyangBody_SERVOMIN, HuyangBody_SERVOMAX);

	// Set the PWM output for the specified servo pin
	_pwm->setPWM(servo, 0, pulselength);
}

// Main loop for HuyangBody, called repeatedly from system.h
void HuyangBody::loop()
{
	_currentMillis = millis(); // Get current time

	// Reset previousMillis if system time rolls over
	if (_previousMillis > _currentMillis)
	{
		_previousMillis = _currentMillis;
	}

	// If in automatic mode, trigger random movements
	if (automatic == true)
	{
		doRandomRotate();
		doRandomTiltForward();
		doRandomTiltSideways();
	}

	updateChestLights(); // Continuously update chest lights based on current mode
}

// --- Body Movement Control Functions ---

// Controls body sideways tilt
void HuyangBody::tiltBodySideways(int16_t degree)
{
	// Apply calibration offset
	degree += calibrationTiltSideways;

	// These mapping values are highly specific to your robot's physical design and servo limits.
	// `calibrationShift` is an existing offset in your original code.
	int16_t calibrationShift = -22; // max 0 to -60
	int16_t calibratedDegree = map(degree, -100, 100, -60 + calibrationShift, 60 + calibrationShift);
	uint16_t rotateDegree = map(calibratedDegree, -100, 100, 0, 170);

	// Apply to left and right servos, potentially with small offsets for symmetry
	rotateServo(pwm_pin_sideway_left, rotateDegree + 0);
	rotateServo(pwm_pin_sideway_right, rotateDegree + 15); // Original code had +15
}

// Controls body forward/backward tilt
void HuyangBody::tiltBodyForward(int16_t degree)
{
	// Apply calibration offset
	degree += calibrationTiltForward;

	// `calibrationShift` is an existing offset in your original code.
	int16_t calibrationShift = 40; // max 0 to 40
	uint16_t rotateDegree = map(degree, -100, 100, 30 + calibrationShift, 140 + calibrationShift);

	// Apply to left and right servos, note the 180 - rotateDegree for right servo for opposing motion
	rotateServo(pwm_pin_forward_left, rotateDegree);
	rotateServo(pwm_pin_forward_right, 180 - rotateDegree);
}

// Controls body rotation (hip/torso rotation)
void HuyangBody::rotateBody(int16_t degree)
{
	// Apply calibration offset
	degree += calibrationRotate;

	// `calibrationShift` is an existing offset in your original code.
	int16_t calibrationShift = 0; // max 0 to 20
	uint16_t rotateDegree = map(degree, -100, 100, 0 + calibrationShift, 70 + calibrationShift);
	rotateServo(pwm_pin_body_rotate, rotateDegree);
}

// Sets all body servos to their predefined center positions
void HuyangBody::centerAll()
{
	// These values (0) should ideally correspond to the mechanical center
	// before any calibration offsets are applied.
	tiltBodySideways(0);
	delay(500); // Small delay to allow servos to reach position
	tiltBodyForward(0);
	delay(500);
	rotateBody(0);
}

// --- Random Movement Functions (for automatic mode) ---

void HuyangBody::doRandomRotate()
{
	if (_randomDoRotate == 0)
	{
		_randomDoRotate = _currentMillis + 2000 + (random(6, 12 + 1) * 1000);
	}

	if (_currentMillis > _randomDoRotate)
	{
		_randomDoRotate = 0;

		if (random(0, 2) == 0) // Randomly choose direction
		{
			rotateBody(-(random(10, 80 + 1))); // Rotate left
		}
		else
		{
			rotateBody(random(10, 80 + 1)); // Rotate right
		}
	}
}

void HuyangBody::doRandomTiltForward()
{
	if (_randomDoTiltForward == 0)
	{
		_randomDoTiltForward = _currentMillis + 2500 + (random(6, 12 + 1) * 1050);
	}

	if (_currentMillis > _randomDoTiltForward)
	{
		_randomDoTiltForward = 0;

		if (random(0, 2) == 0) // Randomly choose direction
		{
			tiltBodyForward(-(random(10, 80 + 1))); // Tilt forward
		}
		else
		{
			tiltBodyForward(random(10, 80 + 1)); // Tilt backward
		}
	}
}

void HuyangBody::doRandomTiltSideways()
{
	if (_randomDoTiltSideways == 0)
	{
		_randomDoTiltSideways = _currentMillis + 3000 + (random(5, 10 + 1) * 1100);
	}

	if (_currentMillis > _randomDoTiltSideways)
	{
		_randomDoTiltSideways = 0;

		if (random(0, 2) == 0) // Randomly choose direction
		{
			tiltBodySideways(-(random(10, 80 + 1))); // Tilt left
		}
		else
		{
			tiltBodySideways(random(10, 80 + 1)); // Tilt right
		}
	}
}

// --- NEW: Chest Light Control Functions ---

// Main function to update chest light behavior based on currentLightMode
void HuyangBody::updateChestLights()
{
	switch (currentLightMode)
	{
	case LIGHT_OFF:
		setAllLights(0); // Turn all pixels off (black color)
		break;
	case LIGHT_ON:
		setAllLights(_neoPixelLights->Color(0, 255, 0)); // Green color for ON (adjust as desired)
		break;
	case LIGHT_BLINK_ALTERNATE:
		// Blink alternating logic
		if (_currentMillis - _lastLightToggleMillis > _blinkInterval)
		{
			_lastLightToggleMillis = _currentMillis;

			// Toggle between patterns: LED0 ON, LED1 OFF -> LED0 OFF, LED1 ON
			if (_neoPixelLights->getPixelColor(0) == _neoPixelLights->Color(0, 255, 0)) // If first LED is green
			{
				setLight(0, 0); // Turn first LED off
				setLight(1, _neoPixelLights->Color(0, 255, 0)); // Turn second LED green
			}
			else
			{
				setLight(0, _neoPixelLights->Color(0, 255, 0)); // Turn first LED green
				setLight(1, 0); // Turn second LED off
			}
		}
		break;
	}
	_neoPixelLights->show(); // Update the NeoPixels to reflect the changes
}

// Sets a single NeoPixel to a specified color
void HuyangBody::setLight(uint8_t pixelNum, uint32_t color)
{
	if (pixelNum < NEO_PIXEL_COUNT)
	{
		_neoPixelLights->setPixelColor(pixelNum, color);
	}
}

// Sets all NeoPixels to a specified color
void HuyangBody::setAllLights(uint32_t color)
{
	for (uint8_t i = 0; i < NEO_PIXEL_COUNT; i++)
	{
		_neoPixelLights->setPixelColor(i, color);
	}
}
