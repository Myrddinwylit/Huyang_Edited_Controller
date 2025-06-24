#include "HuyangBody.h"
#include <Adafruit_PWMServoDriver.h>

// Modified constructor to accept WebServer pointer
HuyangBody::HuyangBody(Adafruit_PWMServoDriver *pwm, WebServer *webserver)
{
	_pwm = pwm;
	_webserver = webserver; // Initialize the WebServer pointer

	_neoPixelLights = new Adafruit_NeoPixel(2, 0, pixelFormat); // Assuming 2 LEDs, pin 0
	_neoPixelLights->setBrightness(20);                          // Default brightness
	_neoPixelLights->begin();
}

void HuyangBody::rotateServo(uint8_t servo, uint16_t degree)
{
	// Map the degree (0-180) to the corresponding pulselength for the servo
	uint16_t pulselength = map(degree, 0, 180, HuyangBody_SERVOMIN, HuyangBody_SERVOMAX);

	// Debugging output (can be uncommented if needed)
	// Serial.print("HuyangBody rotateServo: ");
	// Serial.print(servo);
	// Serial.print(" to Degree: ");
	// Serial.print(degree);
	// Serial.print(" with Pulselength: ");
	// Serial.println(pulselength);

	_pwm->setPWM(servo, 0, pulselength);
}

// Modified tiltBodySideways to apply calibration offset
void HuyangBody::tiltBodySideways(int16_t degree, int16_t calibrationOffset)
{
	// Apply the calibration offset from the WebServer object
	int16_t appliedCalibrationShift = _webserver->calBodyTiltSideways; 

	// Combine incoming degree with offset, then map to servo range.
	// The original 'calibrationShift' was a hardcoded value, now replaced by the dynamic one.
	int16_t calibratedDegree = map(degree, -100, 100, -60 + appliedCalibrationShift, 60 + appliedCalibrationShift);
	uint16_t rotateDegree = map(calibratedDegree, -100, 100, 0, 170); // Map to actual servo pulse range

	rotateServo(pwm_pin_sideway_left, rotateDegree + 0);
	rotateServo(pwm_pin_sideway_right, rotateDegree + 15); // Slight offset for right servo
}

// Modified tiltBodyForward to apply calibration offset
void HuyangBody::tiltBodyForward(int16_t degree, int16_t calibrationOffset)
{
	// Apply the calibration offset from the WebServer object
	int16_t appliedCalibrationShift = _webserver->calBodyTiltForward;

	// Combine incoming degree with offset, then map to servo range.
	int16_t calibratedDegree = map(degree, -100, 100, 30 + appliedCalibrationShift, 140 + appliedCalibrationShift);
	uint16_t rotateDegree = map(calibratedDegree, -100, 100, 0, 180); // Map to actual servo pulse range

	rotateServo(pwm_pin_forward_left, rotateDegree);
	rotateServo(pwm_pin_forward_right, 180 - rotateDegree); // Inverted movement for right servo
}

// Modified rotateBody to apply calibration offset
void HuyangBody::rotateBody(int16_t degree, int16_t calibrationOffset)
{
	// Apply the calibration offset from the WebServer object
	int16_t appliedCalibrationShift = _webserver->calBodyRotation;

	// Combine incoming degree with offset, then map to servo range.
	int16_t calibratedDegree = map(degree, -100, 100, 0 + appliedCalibrationShift, 70 + appliedCalibrationShift);
	uint16_t rotateDegree = map(calibratedDegree, -100, 100, 0, 180); // Map to actual servo pulse range
	
	rotateServo(pwm_pin_body_rotate, rotateDegree);
}

void HuyangBody::centerAll()
{
	// Call the movement functions with 0 degree and apply current calibration offsets
	// This will move the body to its calibrated "centered" position.
	tiltBodySideways(0, _webserver->calBodyTiltSideways);
	delay(500); // Small delay to allow movement

	tiltBodyForward(0, _webserver->calBodyTiltForward);
	delay(500);

	rotateBody(0, _webserver->calBodyRotation);
	delay(500);
}

// New: Implementation for setTorsoLightMode
void HuyangBody::setTorsoLightMode(uint16_t mode)
{
	if (_currentLightMode != mode)
	{
		_currentLightMode = mode;
		_lastLightChangeMillis = _currentMillis; // Reset timer on mode change
		_lightState = false; // Reset blink state
		_lightBlinkInterval = 0; // Reset interval
		
		// Immediately set initial state for new mode
		if (mode == WebServer::LightModeOn) {
			_neoPixelLights->fill(Adafruit_NeoPixel::Color(0, 255, 0), 0, 2); // Green for "On"
			_neoPixelLights->show();
		} else if (mode == WebServer::LightModeOff) {
			_neoPixelLights->clear();
			_neoPixelLights->show();
		} else {
			// For blinking/pattern modes, clear initially and let loop manage
			_neoPixelLights->clear();
			_neoPixelLights->show();
		}
		Serial.printf("HuyangBody: Torso Light Mode set to %d\n", mode);
	}
}

// New: Helper for police lights pattern
void HuyangBody::updatePoliceLights() {
    if (_currentMillis - _lastLightChangeMillis >= 200) { // Toggle every 200ms
        _lastLightChangeMillis = _currentMillis;
        _lightState = !_lightState; // Toggle state

        if (_lightState) {
            // One red, one blue
            _neoPixelLights->setPixelColor(0, Adafruit_NeoPixel::Color(255, 0, 0)); // Red
            _neoPixelLights->setPixelColor(1, Adafruit_NeoPixel::Color(0, 0, 255)); // Blue
        } else {
            // Inverted
            _neoPixelLights->setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 255)); // Blue
            _neoPixelLights->setPixelColor(1, Adafruit_NeoPixel::Color(255, 0, 0)); // Red
        }
        _neoPixelLights->show();
    }
}

// New: Helper for disco lights pattern (random colors)
void HuyangBody::updateDiscoLights() {
    if (_currentMillis - _lastLightChangeMillis >= 150) { // Change every 150ms
        _lastLightChangeMillis = _currentMillis;
        // Set random colors for each pixel
        _neoPixelLights->setPixelColor(0, _neoPixelLights->Color(random(256), random(256), random(256)));
        _neoPixelLights->setPixelColor(1, _neoPixelLights->Color(random(256), random(256), random(256)));
        _neoPixelLights->show();
    }
}

// New: Helper for general blinking lights (slow/fast)
void HuyangBody::updateBlinkLights() {
    if (_currentMillis - _lastLightChangeMillis >= _lightBlinkInterval) {
        _lastLightChangeMillis = _currentMillis;
        _lightState = !_lightState; // Toggle state

        if (_lightState) {
            _neoPixelLights->fill(Adafruit_NeoPixel::Color(255, 255, 0), 0, 2); // Yellow when on
        } else {
            _neoPixelLights->clear(); // Off
        }
        _neoPixelLights->show();
    }
}

void HuyangBody::setup()
{
    // Initialize default body position, applying initial calibration offsets
    // This calls centerAll, which now uses calibration data.
    centerAll(); 
}

void HuyangBody::loop()
{
	_currentMillis = millis(); // Update global currentMillis

	// Original timing for chest lights, now managed by light mode
	// if (_currentMillis - _previousMillis > 100)
	// {
	// 	_previousMillis = _currentMillis;
	// 	updateChestLights(); // This function will now dispatch based on _currentLightMode
	// }

	// New: Handle torso light patterns based on current mode
	switch (_currentLightMode) {
		case WebServer::LightModeOn:
			// Continuously on, set in setTorsoLightMode, no ongoing loop updates needed unless animation
			// For static color, set it once in setTorsoLightMode.
			break;
		case WebServer::LightModeOff:
			// Continuously off, set in setTorsoLightMode, no ongoing loop updates needed.
			break;
		case WebServer::LightModeBlinkSlow:
			_lightBlinkInterval = 1000; // 1 second on/off
			updateBlinkLights();
			break;
		case WebServer::LightModeBlinkFast:
			_lightBlinkInterval = 300; // 0.3 seconds on/off
			updateBlinkLights();
			break;
		case WebServer::LightModePolice:
			updatePoliceLights();
			break;
		case WebServer::LightModeDisco:
			updateDiscoLights();
			break;
		default:
			// Default to off if an unknown mode is set
			_neoPixelLights->clear();
			_neoPixelLights->show();
			break;
	}
}
