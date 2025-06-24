#include "WebServer.h"

// Define the file path for calibration data
#define CALIBRATION_FILE "/calibration.json"

// Set to true to format LittleFS if mount fails. Be cautious as this erases all files.
// For ESP8266, LittleFS.begin() typically does not take a boolean argument for formatting.
// Formatting is usually done via a separate function or tool.
#define FORMAT_LITTLEFS_IF_FAILED false 

WebServer::WebServer(uint32_t port)
{
	_server = new AsyncWebServer(port);
}

// Full regeneration of the setup function and its assignments.
// Parameter names are now prefixed with 'p_' to force a completely fresh parse
// and help eliminate any lingering hidden character issues.
void WebServer::setup(
    bool p_enableEyes,
    bool p_enableMonocle,     // Renamed parameter to ensure uniqueness and clean parsing
    bool p_enableNeckMovement,
    bool p_enableHeadRotation,
    bool p_enableBodyMovement,
    bool p_enableBodyRotation,
    bool p_enableTorsoLights
)
{
	// Assign parameters to class member variables.
	// Using 'this->' explicitly for clarity.
	this->_enableEyes = p_enableEyes;
	this->_enableMonocle = p_enableMonocle; // Direct assignment using the new parameter name
	this->_enableNeckMovement = p_enableNeckMovement;
	this->_enableHeadRotation = p_enableHeadRotation;
	this->_enableBodyMovement = p_enableBodyMovement;
	this->_enableBodyRotation = p_enableBodyRotation;
	this->_enableTorsoLights = p_enableTorsoLights;

	// Initialize LittleFS filesystem
	// For ESP8266, LittleFS.begin() typically takes no arguments to mount.
	// If it fails, report an error.
	if (!LittleFS.begin())
	{
		Serial.println("LittleFS Mount Failed!");
		// If you explicitly need to format the filesystem, it should be done with
		// LittleFS.format() typically as a separate step or a one-time operation.
		return; // Stop setup if filesystem cannot be mounted
	}
	Serial.println("LittleFS mounted successfully.");
	
	// Load existing calibration data from LittleFS on startup
	loadCalibration();
}

String WebServer::readFile(const char *path)
{
	String result = "";
	Serial.printf("Reading file: %s\r\n", path);

	File file = LittleFS.open(path, "r");
	if (!file || file.isDirectory())
	{
		Serial.println("- failed to open file for reading or it's a directory");
		return result;
	}

	result = file.readString();
	file.close();
	Serial.printf("- file size: %d bytes\r\n", result.length());
	return result;
}

void WebServer::writeFile(const char *path, const char *message)
{
	Serial.printf("Writing file: %s\\r\\n", path);

	File file = LittleFS.open(path, "w");
	if (!file)
	{
		Serial.println("- failed to open file for writing");
		return;
	}
	if (file.print(message))
	{
		Serial.println("- file written successfully");
	}
	else
	{
		Serial.println("- write failed");
	}
	file.close();
}

// New: Implementation for setServosToMiddleAndLock
void WebServer::setServosToMiddleAndLock() {
    Serial.println("WebServer: Setting servos to middle and locking for calibration.");
    servosLockedForCalibration = true;
    // This flag will be read by the main loop in system.h to control servo behavior.
}

// New: Implementation for unlockServos
void WebServer::unlockServos() {
    Serial.println("WebServer: Unlocking servos, returning to normal operation.");
    servosLockedForCalibration = false;
    // This flag will be read by the main loop in system.h to resume normal control.
}

// New: Implementation for resetCalibrationToDefaults
void WebServer::resetCalibrationToDefaults() {
    Serial.println("WebServer: Resetting calibration to defaults (0).");
    calNeckRotation = 0;
    calNeckTiltForward = 0;
    calNeckTiltSideways = 0;
    calBodyRotation = 0;
    calBodyTiltForward = 0;
    calBodyTiltSideways = 0;
    calMonoclePosition = 0;
    saveCalibration(); // Immediately save the reset values to persistent storage
}

// New: Implementation for loadCalibration
void WebServer::loadCalibration() {
    Serial.println("WebServer: Loading calibration data...");
    String jsonString = readFile(CALIBRATION_FILE);

    if (jsonString.length() == 0) {
        Serial.println("- No calibration file found or empty. Using default 0s.");
        resetCalibrationToDefaults(); // Initialize with defaults if no file or empty file
        return;
    }

    // Use ArduinoJson to parse the calibration data
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        Serial.println("- Failed to parse calibration JSON. Using default 0s.");
        resetCalibrationToDefaults(); // Reset on parsing error
        return;
    }

    // Extract calibration values, using a default of 0 if a key is missing
    JsonObject neckCal = doc["neck"];
    calNeckRotation = neckCal["rotation"] | 0;
    calNeckTiltForward = neckCal["tiltForward"] | 0;
    calNeckTiltSideways = neckCal["tiltSideways"] | 0;

    JsonObject bodyCal = doc["body"];
    calBodyRotation = bodyCal["rotation"] | 0;
    calBodyTiltForward = bodyCal["tiltForward"] | 0;
    calBodyTiltSideways = bodyCal["tiltSideways"] | 0;

    JsonObject monocleCal = doc["monocle"];
    calMonoclePosition = monocleCal["position"] | 0;

    Serial.println("- Calibration data loaded:");
    Serial.printf("  Neck: R:%d, TF:%d, TS:%d\n", calNeckRotation, calNeckTiltForward, calNeckTiltSideways);
    Serial.printf("  Body: R:%d, TF:%d, TS:%d\n", calBodyRotation, calBodyTiltForward, calBodyTiltSideways);
    Serial.printf("  Monocle: P:%d\n", calMonoclePosition);
}

// New: Implementation for saveCalibration
void WebServer::saveCalibration() {
    Serial.println("WebServer: Saving calibration data...");
    JsonDocument doc;
    
    // Populate the JSON document with current calibration values
    serializeCalibrationJson(doc);

    String jsonString;
    serializeJson(doc, jsonString); // Serialize to a string

    writeFile(CALIBRATION_FILE, jsonString.c_str()); // Write the JSON string to file
    Serial.println("- Calibration data saved.");
}

// New: Implementation for serializeCalibrationJson helper
void WebServer::serializeCalibrationJson(JsonDocument& doc) {
    // Populate the JSON object with current calibration values
    doc["neck"]["rotation"] = calNeckRotation;
    doc["neck"]["tiltForward"] = calNeckTiltForward;
    doc["neck"]["tiltSideways"] = calNeckTiltSideways;

    doc["body"]["rotation"] = calBodyRotation;
    doc["body"]["tiltForward"] = calBodyTiltForward;
    doc["body"]["tiltSideways"] = calBodyTiltSideways;

    doc["monocle"]["position"] = calMonoclePosition;
}


void WebServer::start()
{
	// Route for handling POST requests with JSON data from client
	_server->on(
		"/api/post.json", HTTP_POST, [&](AsyncWebServerRequest *request) {}, nullptr, [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
		{ apiPostAction(request, data, len, index, total); });

	// Serve static files from LittleFS
	_server->on("/styles.css", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(LittleFS, "/styles.css", "text/css"); });
	_server->on("/javascript.js", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(LittleFS, "/javascript.js", "text/javascript"); });
	_server->on("/joystick.js", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(LittleFS, "/joystick.js", "text/javascript"); });

	// Serve HTML pages, dynamically generated if needed via getPage() or directly from LittleFS
	_server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(200, "text/html", getPage(indexPage, request)); });
	_server->on("/index.html", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(200, "text/html", getPage(indexPage, request)); });
	_server->on("/index.face.html", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(LittleFS, "/index.face.html", "text/html"); });
	_server->on("/index.neck.html", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(LittleFS, "/index.neck.html", "text/html"); });
	_server->on("/index.body.html", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(LittleFS, "/index.body.html", "text/html"); });
	_server->on("/settings.html", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(200, "text/html", getPage(settingsPage, request)); });
	_server->on("/calibration.html", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(200, "text/html", getPage(calibrationPage, request)); });
	
	// New: Route for chestlights.html
	_server->on("/chestlights.html", HTTP_GET, [&](AsyncWebServerRequest *request) { request->send(200, "text/html", getPage(chestLightsPage, request)); });


	_server->onNotFound([&](AsyncWebServerRequest *request)
						{ notFound(request); });

	_server->begin();
}

// ADDED: Definition for the WebServer's loop method
void WebServer::loop() {
    // If you have any ongoing tasks for the web server that need to be run
    // periodically (e.g., handling WebSocket events, or other background tasks),
    // they would go here. For AsyncWebServer, much of the request handling is
    // asynchronous and event-driven, so this might remain empty or contain
    // specific service-related logic if you add more complex features.
    // For now, it's a placeholder to satisfy the linker.
}

void WebServer::apiPostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	Serial.println("apiPostAction: Received data.");

	// Use a StaticJsonDocument for efficiency if size is predictable, or DynamicJsonDocument
	// for larger/variable sizes. 512 bytes should be enough for typical control messages.
	StaticJsonDocument<512> jsonBuffer; 
	DeserializationError error = deserializeJson(jsonBuffer, data, len);

	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.f_str());
		request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
		return;
	}

	// Make jsonBuffer a JsonObject for easier access
	JsonObject json = jsonBuffer.as<JsonObject>();

	// Process automatic animations toggle
	if (!json["automatic"].isNull())
	{
		automaticAnimations = json["automatic"].as<bool>();
		Serial.print("post: automatic: ");
		Serial.println(automaticAnimations ? "true" : "false");

		// When automatic is enabled, reset eye states to default blink for visual consistency
		if (automaticAnimations)
		{
			allEyes = 0;
			leftEye = 3; // Blink
			rightEye = 3; // Blink
		}
	}

	// Process face commands
	if (json.containsKey("face") && !json["face"].isNull())
	{
		JsonObject faceJson = json["face"].as<JsonObject>();
		
		// If an 'all' eye command is present, prioritize it
		if (faceJson.containsKey("all") && !faceJson["all"].isNull())
		{
			allEyes = faceJson["all"].as<uint16_t>();
			leftEye = 0; // Reset individual eye commands
			rightEye = 0;
			automaticAnimations = false; // Manual control takes precedence
			Serial.printf("post: allEyes: %d\n", allEyes);
		}
		else // Otherwise, check for individual eye commands
		{
			allEyes = 0; // Clear 'all' command if individual eyes are controlled

			if (faceJson.containsKey("left") && !faceJson["left"].isNull())
			{
				leftEye = faceJson["left"].as<uint16_t>();
				automaticAnimations = false;
				Serial.printf("post: leftEye: %d\n", leftEye);
			}
			if (faceJson.containsKey("right") && !faceJson["right"].isNull())
			{
				rightEye = faceJson["right"].as<uint16_t>();
				automaticAnimations = false;
				Serial.printf("post: rightEye: %d\n", rightEye);
			}
		}
	}

	// Process neck commands
	if (json.containsKey("neck") && !json["neck"].isNull())
	{
		JsonObject neckJson = json["neck"].as<JsonObject>();
		if (neckJson.containsKey("rotate") && !neckJson["rotate"].isNull())
		{
			neckRotate = neckJson["rotate"].as<int16_t>();
			automaticAnimations = false;
			Serial.printf("post: neckRotate: %d\n", neckRotate);
		}
		if (neckJson.containsKey("tiltForward") && !neckJson["tiltForward"].isNull())
		{
			neckTiltForward = neckJson["tiltForward"].as<int16_t>();
			automaticAnimations = false;
			Serial.printf("post: neckTiltForward: %d\n", neckTiltForward);
		}
		if (neckJson.containsKey("tiltSideways") && !neckJson["tiltSideways"].isNull())
		{
			neckTiltSideways = neckJson["tiltSideways"].as<int16_t>();
			automaticAnimations = false;
			Serial.printf("post: neckTiltSideways: %d\n", neckTiltSideways);
		}
	}

	// Process body commands
	if (json.containsKey("body") && !json["body"].isNull())
	{
		JsonObject bodyJson = json["body"].as<JsonObject>();
		if (bodyJson.containsKey("rotate") && !bodyJson["rotate"].isNull())
		{
			bodyRotate = bodyJson["rotate"].as<int16_t>();
			automaticAnimations = false;
			Serial.printf("post: bodyRotate: %d\n", bodyRotate);
		}
		if (bodyJson.containsKey("tiltForward") && !bodyJson["tiltForward"].isNull())
		{
			bodyTiltForward = bodyJson["tiltForward"].as<int16_t>();
			automaticAnimations = false;
			Serial.printf("post: bodyTiltForward: %d\n", bodyTiltForward);
		}
		if (json.containsKey("tiltSideways") && !json["tiltSideways"].isNull())
		{
			bodyTiltSideways = json["tiltSideways"].as<int16_t>();
			automaticAnimations = false;
			Serial.printf("post: bodyTiltSideways: %d\n", bodyTiltSideways);
		}
	}

	// New: Process torso light commands
	if (json.containsKey("torso") && !json["torso"].isNull()) {
		JsonObject torsoJson = json["torso"].as<JsonObject>();
		if (torsoJson.containsKey("lightMode") && !torsoJson["lightMode"].isNull()) {
			currentTorsoLightMode = torsoJson["lightMode"].as<uint16_t>();
			automaticAnimations = false; // Light changes are manual commands
			Serial.printf("post: currentTorsoLightMode: %d\n", currentTorsoLightMode);
		}
	}

	// New: Process calibration commands
	if (json.containsKey("calibration") && !json["calibration"].isNull()) {
		JsonObject calJson = json["calibration"].as<JsonObject>();

		if (calJson.containsKey("action") && !calJson["action"].isNull()) {
			String action = calJson["action"].as<String>();
			if (action == "setMiddleAndLock") {
				setServosToMiddleAndLock();
				automaticAnimations = false; // Calibration is always manual
			} else if (action == "unlockServos") {
				unlockServos();
				automaticAnimations = false;
			} else if (action == "reset") {
				resetCalibrationToDefaults();
				automaticAnimations = false;
			} else if (action == "save") {
				saveCalibration();
				automaticAnimations = false;
			}
		} else if (calJson.containsKey("part") && !calJson["part"].isNull() &&
				   calJson.containsKey("param") && !calJson["param"].isNull() &&
				   calJson.containsKey("value") && !calJson["value"].isNull()) {
			
			String part = calJson["part"].as<String>();
			String param = calJson["param"].as<String>();
			int16_t value = calJson["value"].as<int16_t>();
			
			if (part == "neck") {
				if (param == "rotation") calNeckRotation = value;
				else if (param == "tiltForward") calNeckTiltForward = value;
				else if (param == "tiltSideways") calNeckTiltSideways = value;
			} else if (part == "body") {
				if (param == "rotation") calBodyRotation = value;
				else if (param == "tiltForward") calBodyTiltForward = value;
				else if (param == "tiltSideways") calBodyTiltSideways = value;
			} else if (part == "monocle") {
				if (param == "position") calMonoclePosition = value;
			}
			automaticAnimations = false; // Calibration is a manual adjustment
			Serial.printf("post: calibration %s.%s: %d\n", part.c_str(), param.c_str(), value);
		}
	}


	// Construct response JSON
	JsonDocument responseDoc; // Use a new document for the response
	responseDoc["automatic"] = automaticAnimations;

	// Always send back current state for UI consistency
	// Face
	responseDoc["face"]["all"] = allEyes;
	responseDoc["face"]["left"] = leftEye;
	responseDoc["face"]["right"] = rightEye;

	// Neck
	responseDoc["neck"]["rotate"] = neckRotate;
	responseDoc["neck"]["tiltForward"] = neckTiltForward;
	responseDoc["neck"]["tiltSideways"] = neckTiltSideways;

	// Body
	responseDoc["body"]["rotate"] = bodyRotate;
	responseDoc["body"]["tiltForward"] = bodyTiltForward;
	responseDoc["body"]["tiltSideways"] = bodyTiltSideways;

	// Torso Lights
	responseDoc["torso"]["lightMode"] = currentTorsoLightMode;

	// Calibration data
	serializeCalibrationJson(responseDoc); // Use the new helper to add calibration data to response

	String result;
	serializeJson(responseDoc, result); // Serialize to a string

	request->send(200, "application/json", result);
}

String WebServer::getPage(Page page, AsyncWebServerRequest *request)
{
	String pageContent = ""; // Use a local variable for content
	
	switch (page)
	{
	case indexPage:
	{
		pageContent = readFile("/index.html");
		getBaseHtml(pageContent, _html); // Populate _html with base content and index content

		if (_enableEyes)
		{
			_html.replace("###FACE###", "<div id=\"container_face\" onclick=\"loadContainer('/index.face.html', 'container_face')\">Activate Face Interface</div>");
		}
		else
		{
			_html.replace("###FACE###", "");
		}

		if (_enableNeckMovement || _enableHeadRotation)
		{
			_html.replace("###NECK###", "<div id=\"container_neck\" onclick=\"loadContainer('/index.neck.html', 'container_neck')\">Activate Neck Interface</div>");
		}
		else
		{
			_html.replace("###NECK###", "");
		}

		if (_enableBodyMovement || _enableBodyRotation)
		{
			_html.replace("###BODY###", "<div id=\"container_body\" onclick=\"loadContainer('/index.body.html', 'container_body')\">Activate Body Interface</div>");
		}
		else
		{
			_html.replace("###BODY###", "");
		}
	}
	break;
	case settingsPage:
	{
		pageContent = readFile("/settings.html");
		getBaseHtml(pageContent, _html);
	}
	break;
	case calibrationPage:
	{
		pageContent = readFile("/calibration.html");
		getBaseHtml(pageContent, _html);
	}
	break;
	// New: Handle chestLightsPage
	case chestLightsPage:
	{
		pageContent = readFile("/chestlights.html"); // Assuming you'll have a chestlights.html file
		getBaseHtml(pageContent, _html);
	}
	break;
	}
	return _html;
}

void WebServer::notFound(AsyncWebServerRequest *request)
{
	request->send(404, "text/plain", "Not found");
}

void WebServer::getBaseHtml(const String &body, String &target)
{
	target = readFile("/base.html");
	target.replace("###BODY###", body);
}
