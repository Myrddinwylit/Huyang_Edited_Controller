#ifndef HuyangFace_h
#define HuyangFace_h

#include "Arduino.h"
#include <Arduino_GFX_Library.h>

#define tftColor(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

class HuyangFace
{
public:
	enum EyeState
	{
		None = 0,
		Open = 1,
		Closed = 2,
		Blink = 3,
		Focus = 4,
		Sad = 5,
		Angry = 6
	};

	HuyangFace(Arduino_GFX *left, Arduino_GFX *right);

	void setup();
	void loop();

	bool automatic = true;

	void setEyesTo(EyeState newState);
	void setLeftEyeTo(EyeState newState);
	void setRightEyeTo(EyeState newState);

	// Moved to public as per original implementation logic from HuyangFace.cpp
	EyeState getStateFrom(uint8_t state); 

private:
	Arduino_GFX *_leftEye;
	Arduino_GFX *_rightEye;

	unsigned long _currentMillis = 0;
	unsigned long _previousMillis = 0;
	unsigned long _previousRandomMillis = 0;
	
	uint16_t _tftDisplayWidth = 240;
	uint16_t _tftDisplayHeight = 240;

	// These are your original color definitions as private member variables
	uint16_t _huyangEyeColor = tftColor(255 - 255, 255 - 221, 255 - 34); // 0xFD20
	uint16_t _huyangClosedEyeColor = tftColor(255, 255, 255); // Original white

	EyeState _leftEyeLastSelectedState = Blink;
	EyeState _rightEyeLastSelectedState = Blink;

	EyeState _leftEyeTargetState = Blink;
	EyeState _rightEyeTargetState = Blink;

	EyeState _leftEyeState = Closed; // Original state tracking
	EyeState _rightEyeState = Closed; // Original state tracking
	
	uint32_t _randomDuration = 2000;
	uint16_t _blinkDelay = 1;

	// Original drawing loop functions declared here
	void openEyesLoop();
	void openEyes(uint16_t color); // This function will now clear the screen first
	void openEye(Arduino_GFX *eye, uint16_t color); // This function will now clear the screen first

	void closeEyesLoop();
	void closeEyes(uint16_t color);
	void closeEye(Arduino_GFX *eye, uint16_t color);

	void focusEyesLoop();
	void focusEyes(uint16_t color); // This function will now clear the screen first
	void focusEye(Arduino_GFX *eye, uint16_t color); // This function will now clear the screen first

	void sadEyesLoop();
	void sadEyes(uint16_t color); // This function will now clear the screen first
	void sadEye(Arduino_GFX *eye, bool inner, uint16_t color); // This function will now clear the screen first

	void angryEyesLoop();
	void angryEyes(uint16_t color); // This function will now clear the screen first
	// Note: angryEye helper was not in original HuyangFace.h, so not re-adding.
	// If it exists in _moods.cpp, it should also clear if called directly.
};

#endif
