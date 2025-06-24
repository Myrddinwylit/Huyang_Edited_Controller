#ifndef WebServer_h
#define WebServer_h

#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif

#include "FS.h"
#include "LittleFS.h"


#include <ArduinoJson.h>

// #include "content/styles.h" // Commented out in original, keeping it that way
// #include "content/javascript.h" // Commented out in original, keeping it that way
// #include "content/joystick.h" // Commented out in original, keeping it that way
// #include "content/baseHtml.h" // Commented out in original, keeping it that way

// #include "pages/indexHtml.h" // Commented out in original, keeping it that way
// #include "pages/settingsHtml.h" // Commented out in original, keeping it that way
// #include "pages/calibrationHtml.h" // Commented out in original, keeping it that way


class WebServer
{
public:
	enum Page
	{
		indexPage,
		settingsPage,
		calibrationPage,
		chestLightsPage // New: Enum for the chest lights page
	};

	// Public variables to store control states and calibration values
	bool automaticAnimations = true;

	uint16_t leftEye = 3; // Corresponds to HuyangFace::EyeState::Blink
	uint16_t rightEye = 3; // Corresponds to HuyangFace::EyeState::Blink
	uint16_t allEyes = 0; // 0 means no 'all' command, individual eyes used

	int16_t neckRotate = 0;
	int16_t neckTiltForward = 0;
	int16_t neckTiltSideways = 0;

	int16_t bodyRotate = 0;
	int16_t bodyTiltForward = 0;
	int16_t bodyTiltSideways = 0;

	uint16_t currentTorsoLightMode = 0; // New: Current mode for torso lights (0 = off, 1 = on, etc.)

	// Calibration offsets (new)
	int16_t calNeckRotation = 0;
	int16_t calNeckTiltForward = 0;
	int16_t calNeckTiltSideways = 0;
	int16_t calBodyRotation = 0;
	int16_t calBodyTiltForward = 0;
	int16_t calBodyTiltSideways = 0;
	int16_t calMonoclePosition = 0;

	// Flag to indicate if servos should be locked at calibrated middle positions
	bool servosLockedForCalibration = false;

	// Enum for Torso Light Modes (must match client-side JS values)
	enum LightMode {
		LightModeOff = 0,
		LightModeOn = 1,
		LightModeBlinkSlow = 2, // Used for "Warning Blink"
		LightModeBlinkFast = 3,
		LightModePolice = 4,
		LightModeDisco = 5
	};


	WebServer(uint32_t port);
	void setup(bool enableEyes,
			   bool enableMonacle,
			   bool enableNeckMovement,
			   bool enableHeadRotation,
			   bool enableBodyMovement,
			   bool enableBodyRotation,
			   bool enableTorsoLights);
	void start();
    void loop(); // ADDED: Declaration for the loop method

	// New: Public methods for calibration actions
	void setServosToMiddleAndLock();
	void unlockServos();
	void resetCalibrationToDefaults();
	void loadCalibration();
	void saveCalibration();

private:
	AsyncWebServer *_server;
	String _html;

	// Private flags reflecting enabled features (set during setup)
	bool _enableEyes;
	bool _enableMonocle;
	bool _enableNeckMovement;
	bool _enableHeadRotation;
	bool _enableBodyMovement;
	bool _enableBodyRotation;
	bool _enableTorsoLights; // New: Flag for torso lights

	String readFile(const char *path);
	void writeFile(const char *path, const char *message);
	String getPage(Page page, AsyncWebServerRequest *request);
	void notFound(AsyncWebServerRequest *request);
	void apiPostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
	void getBaseHtml(const String &body, String &target);

	// Helper for serializing calibration JSON (private as it's an internal utility)
	void serializeCalibrationJson(JsonDocument& doc);
};

#endif
