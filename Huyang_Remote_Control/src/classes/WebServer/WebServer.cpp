    #include "WebServer.h" // Include the corresponding header file

    // Define the file path for calibration data on LittleFS
    #define CALIBRATION_FILE "/calibrations.json" 

    // --- GLOBAL VARIABLES DEFINITIONS (MATCHING EXTERN DECLARATIONS IN WebServer.h) ---
    // These are the actual definitions of the global state variables.
    // They are used by WebServer to update robot state and by other classes (e.g., HuyangFace, HuyangBody)
    // to control the robot's hardware.

    // Control flags
    bool automaticAnimations = true; // Default to automatic
    uint16_t allEyes = 0; // No specific "all eyes" command active by default
    uint16_t faceLeftEyeState = 3;  // Default to blink (state 3)
    uint16_t faceRightEyeState = 3; // Default to blink (state 3)

    // Neck movement values
    double neckRotate = 0;
    double neckTiltForward = 0;
    double neckTiltSideways = 0;

    // Body movement values
    int16_t bodyRotate = 0;
    int16_t bodyTiltForward = 0;
    int16_t bodyTiltSideways = 0;

    int16_t monoclePosition = 0; // Monocle servo position

    // Calibration values (defaults, loaded from file if present)
    int16_t calNeckRotation = 0;
    int16_t calNeckTiltForward = 0;
    int16_t calNeckTiltSideways = 0;
    int16_t calBodyRotation = 0;
    int16_t calBodyTiltForward = 0;
    int16_t calBodyTiltSideways = 0;
    int16_t calMonoclePosition = 0; 

    // Chest light mode (Default to LIGHT_STATIC_BLUE)
    LightMode chestLightMode = LIGHT_STATIC_BLUE; 

    // Constructor: Initializes the AsyncWebServer with the specified port
    WebServer::WebServer(uint32_t port)
    {
    	_server = new AsyncWebServer(port);
    }

    // Setup method: Initializes LittleFS, loads calibration, and configures web server routes.
    void WebServer::setup(bool enableEyes,
    					  bool enableMonacle,
    					  bool enableNeckMovement,
    					  bool enableHeadRotation,
    					  bool enableBodyMovement,
    					  bool enableBodyRotation,
    					  bool enableTorsoLights)
    {
    	// Store feature enable flags
    	_enableEyes = enableEyes;
    	_enableMonacle = enableMonacle;
    	_enableNeckMovement = enableNeckMovement;
    	_enableHeadRotation = enableHeadRotation;
    	_enableBodyMovement = enableBodyMovement;
    	_enableBodyRotation = enableBodyRotation;
    	_enableTorsoLights = enableTorsoLights;

    	// Initialize LittleFS file system
    	if (!LittleFS.begin())
    	{
    		Serial.println("LittleFS Mount Failed! Attempting to format...");
    		LittleFS.format(); 
    		if (!LittleFS.begin()) {
    			Serial.println("LittleFS Re-Mount Failed! Cannot proceed, file operations will fail.");
    			return;
    		}
    		Serial.println("LittleFS Formatted and Mounted successfully.");
    	} else {
    		Serial.println("LittleFS Mounted successfully.");
    	}

    	loadCalibration();
    }

    // Helper function to read content from a file on LittleFS.
    String WebServer::readFile(const char *path)
    {
    	String result = "";
    	Serial.printf("Attempting to read file: %s\r\n", path);

    	File file = LittleFS.open(path, "r");
    	if (!file || file.isDirectory())
    	{
    		Serial.printf("- Failed to open file %s for reading or it is a directory.\r\n", path);
    		return ""; 
    	}

    	result = file.readString();
    	file.close();
    	Serial.printf("- Successfully read %d bytes from %s\r\n", result.length(), path);
    	return result;
    }

    // Helper function to write content to a file on LittleFS.
    void WebServer::writeFile(const char *path, const char *message)
    {
    	Serial.printf("Attempting to write file: %s\r\n", path);

    	File file = LittleFS.open(path, "w");
    	if (!file)
    	{
    		Serial.println("- Failed to open file for writing.");
    		return;
    	}
    	if (file.print(message))
    	{
    		Serial.println("- File written successfully.");
    	}
    	else
    	{
    		Serial.println("- Write failed.");
    	}
    	delay(200); 
    	file.close();
    }

    // --- Calibration Load/Save/Reset Functions ---
    void WebServer::loadCalibration() {
    	Serial.println("Loading calibration from LittleFS...");
    	String calibrationJson = readFile(CALIBRATION_FILE); 

    	if (calibrationJson.length() > 0) { 
    		JsonDocument doc;
    		DeserializationError error = deserializeJson(doc, calibrationJson); 

    		if (!error) { 
    			Serial.println("Calibration JSON parsed successfully.");
    			calNeckRotation = doc["neck"]["rotation"] | 0;
    			calNeckTiltForward = doc["neck"]["tiltForward"] | 0;
    			calNeckTiltSideways = doc["neck"]["tiltSideways"] | 0;
    			calBodyRotation = doc["body"]["rotation"] | 0;
    			calBodyTiltForward = doc["body"]["tiltForward"] | 0;
    			calBodyTiltSideways = doc["body"]["tiltSideways"] | 0;
    			calMonoclePosition = doc["monocle"]["position"] | 0; 

    			Serial.printf("Loaded Calibration: Neck R:%d, TF:%d, TS:%d; Body R:%d, TF:%d, TS:%d; Monocle P:%d\n", 
    			              calNeckRotation, calNeckTiltForward, calNeckTiltSideways,
    			              calBodyRotation, calBodyTiltForward, calBodyTiltSideways,
    			              calMonoclePosition);

    		} else {
    			Serial.print("Failed to parse calibration JSON: ");
    			Serial.println(error.c_str());
    			resetCalibrationToDefaults(); 
    		}
    	} else {
    		Serial.println("Calibration file not found or empty. Using default values.");
    		resetCalibrationToDefaults(); 
    	}
    }

    void WebServer::saveCalibration() {
    	Serial.println("Saving calibration to LittleFS...");
    	JsonDocument doc; 
    	doc["neck"]["rotation"] = calNeckRotation;
    	doc["neck"]["tiltForward"] = calNeckTiltForward;
    	doc["neck"]["tiltSideways"] = calNeckTiltSideways;
    	doc["body"]["rotation"] = calBodyRotation;
    	doc["body"]["tiltForward"] = calBodyTiltForward;
    	doc["body"]["tiltSideways"] = calBodyTiltSideways;
    	doc["monocle"]["position"] = calMonoclePosition; 

    	String output;
    	serializeJson(doc, output); 
    	writeFile(CALIBRATION_FILE, output.c_str()); 
    	Serial.println("Calibration saved.");
    }

    void WebServer::resetCalibrationToDefaults() {
    	Serial.println("Resetting calibration to default values (0).");
    	calNeckRotation = 0;
    	calNeckTiltForward = 0;
    	calNeckTiltSideways = 0;
    	calBodyRotation = 0;
    	calBodyTiltForward = 0;
    	calBodyTiltSideways = 0;
    	calMonoclePosition = 0; 
    	saveCalibration(); 
    }

    // Starts the web server and defines all its routes (API endpoints and static files).
    void WebServer::start()
    {
    	_server->on(
    		"/api/post.json", HTTP_POST, 
    		[&](AsyncWebServerRequest *request) {}, 
    		nullptr, 
    		[&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    		{ apiPostAction(request, data, len, index, total); }
    	);

    	_server->on(
    		"/api/calibrate.json", HTTP_POST, 
    		[&](AsyncWebServerRequest *request) {}, 
    		nullptr, 
    		[&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    		{ apiCalibratePostAction(request, data, len, index, total); }
    	);

    	_server->on(
    		"/api/lights.json", HTTP_POST, 
    		[&](AsyncWebServerRequest *request) {}, 
    		nullptr, 
    		[&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    		{ apiLightsPostAction(request, data, len, index, total); }
    	);

    	_server->on("/api/calibration/get.json", HTTP_GET, [&](AsyncWebServerRequest *request){
    		apiGetCalibration(request);
    	});

    	_server->on("/styles.css", HTTP_GET, [&](AsyncWebServerRequest *request)
    				{ request->send(LittleFS, "/styles.css", "text/css"); });
    	_server->on("/javascript.js", HTTP_GET, [&](AsyncWebServerRequest *request)
    				{ request->send(LittleFS, "/javascript.js", "text/javascript"); });
    	_server->on("/joystick.js", HTTP_GET, [&](AsyncWebServerRequest *request)
    				{ request->send(LittleFS, "/joystick.js", "text/javascript"); });

    	_server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request)
    				{ request->send(LittleFS, "/index.html", "text/html"); }); 
    	_server->on("/index.html", HTTP_GET, [&](AsyncWebServerRequest *request)
    				{ request->send(LittleFS, "/index.html", "text/html"); });
    	
    	_server->on("/settings.html", HTTP_GET, [&](AsyncWebServerRequest *request)
    				{ request->send(LittleFS, "/settings.html", "text/html"); });
    	_server->on("/calibration.html", HTTP_GET, [&](AsyncWebServerRequest *request)
    				{ request->send(LittleFS, "/calibration.html", "text/html"); });
    	_server->on("/chestlights.html", HTTP_GET, [&](AsyncWebServerRequest *request) 
    				{ request->send(LittleFS, "/chestlights.html", "text/html"); }); 

    	_server->onNotFound([&](AsyncWebServerRequest *request)
    						{ notFound(request); });

    	_server->begin(); 
    	Serial.println("Web server started.");
    }

    void WebServer::apiPostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    {
    	Serial.println("apiPostAction (General Control) received.");

    	JsonDocument json;
    	DeserializationError error = deserializeJson(json, data, len); 
    	if (error) {
    		Serial.print(F("deserializeJson() failed: "));
    		Serial.println(error.f_str());
    		request->send(400, "text/plain", "Bad Request: Invalid JSON");
    		return;
    	}

    	JsonDocument r; 

    	if (json.containsKey("automatic") && !json["automatic"].isNull())
    	{
    		automaticAnimations = json["automatic"].as<bool>();
    		Serial.printf("post: automatic: %s\n", automaticAnimations ? "true" : "false");
    		if (automaticAnimations)
    		{
    			allEyes = 0;
    			faceLeftEyeState = 3; 
    			faceRightEyeState = 3; 
    		}
    	}

    	if (json.containsKey("face") && !json["face"].isNull())
    	{
    		if (json["face"].containsKey("all") && !json["face"]["all"].isNull())
    		{
    			uint16_t face_all_val = json["face"]["all"].as<uint16_t>();
    			if (face_all_val != 0)
    			{
    				allEyes = face_all_val;
    				faceLeftEyeState = 0; 
    				faceRightEyeState = 0; 
    				automaticAnimations = false; 
    				Serial.printf("post: allEyes: %d\n", allEyes);
    			} else {
    				allEyes = 0; 
    			}
    		}
    		
    		if (json["face"].containsKey("left") && !json["face"]["left"].isNull())
    		{
    			faceLeftEyeState = json["face"]["left"].as<uint16_t>();
    			automaticAnimations = false; 
    			Serial.printf("post: faceLeftEyeState: %d\n", faceLeftEyeState);
    		}
    		if (json["face"].containsKey("right") && !json["face"]["right"].isNull())
    		{
    			faceRightEyeState = json["face"]["right"].as<uint16_t>();
    			automaticAnimations = false; 
    			Serial.printf("post: faceRightEyeState: %d\n", faceRightEyeState);
    		}
            if (json["face"].containsKey("monocle") && !json["face"]["monocle"].isNull()) {
                monoclePosition = json["face"]["monocle"].as<int16_t>();
                automaticAnimations = false;
                Serial.printf("post: monoclePosition: %d\n", monoclePosition);
            }
    	}

    	if (json.containsKey("neck") && !json["neck"].isNull())
    	{
    		if (json["neck"].containsKey("rotate") && !json["neck"]["rotate"].isNull())
    		{
    			neckRotate = json["neck"]["rotate"].as<double>();
    			automaticAnimations = false;
    			Serial.printf("post: neckRotate: %.2f\n", neckRotate);
    		}
    		if (json["neck"].containsKey("tiltForward") && !json["neck"]["tiltForward"].isNull())
    		{
    			neckTiltForward = json["neck"]["tiltForward"].as<double>();
    			automaticAnimations = false;
    			Serial.printf("post: neckTiltForward: %.2f\n", neckTiltForward);
    		}
    		if (json["neck"].containsKey("tiltSideways") && !json["neck"]["tiltSideways"].isNull())
    		{
    			neckTiltSideways = json["neck"]["tiltSideways"].as<double>();
    			automaticAnimations = false;
    			Serial.printf("post: neckTiltSideways: %.2f\n", neckTiltSideways);
    		}
    	}

    	if (json.containsKey("body") && !json["body"].isNull())
    	{
    		if (json["body"].containsKey("rotate") && !json["body"]["rotate"].isNull())
    		{
    			bodyRotate = json["body"]["rotate"].as<int16_t>();
    			automaticAnimations = false;
    			Serial.printf("post: bodyRotate: %d\n", bodyRotate);
    		}
    		if (json["body"].containsKey("tiltForward") && !json["body"]["tiltForward"].isNull())
    		{
    			bodyTiltForward = json["body"]["tiltForward"].as<int16_t>();
    			automaticAnimations = false;
    			Serial.printf("post: bodyTiltForward: %d\n", bodyTiltForward);
    		}
    		if (json["body"].containsKey("tiltSideways") && !json["body"]["tiltSideways"].isNull())
    		{
    			bodyTiltSideways = json["body"]["tiltSideways"].as<int16_t>();
    			automaticAnimations = false;
    			Serial.printf("post: bodyTiltSideways: %d\n", bodyTiltSideways);
    		}
    	}

    	r["automatic"] = automaticAnimations;
    	r["face"]["eyes"]["all"] = allEyes;
    	r["face"]["eyes"]["left"] = faceLeftEyeState; 
    	r["face"]["eyes"]["right"] = faceRightEyeState;
        r["face"]["monocle"]["position"] = monoclePosition; 
    	r["neck"]["rotate"] = neckRotate;
    	r["neck"]["tiltForward"] = neckTiltForward;
    	r["neck"]["tiltSideways"] = neckTiltSideways;
    	r["body"]["rotate"] = bodyRotate;
    	r["body"]["tiltForward"] = bodyTiltForward;
    	r["body"]["tiltSideways"] = bodyTiltSideways;
    	r["chestLightMode"] = (uint8_t)chestLightMode; 

    	String result;
    	serializeJson(r, result); 
    	request->send(200, "application/json", result); 
    	Serial.println("apiPostAction response sent.");
    }

    void WebServer::apiCalibratePostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    	Serial.println("apiCalibratePostAction received.");

    	JsonDocument json;
    	DeserializationError error = deserializeJson(json, data, len); 
    	if (error) {
    		Serial.print(F("deserializeJson() failed: "));
    		Serial.println(error.f_str());
    		request->send(400, "text/plain", "Bad Request: Invalid JSON");
    		return;
    	}

    	if (json.containsKey("neck") && !json["neck"].isNull()) {
    		if (json["neck"].containsKey("rotation") && !json["neck"]["rotation"].isNull()) {
    			calNeckRotation = json["neck"]["rotation"].as<int16_t>();
    			Serial.printf("Calibrating Neck Rotation: %d\n", calNeckRotation);
    		}
    		if (json["neck"].containsKey("tiltForward") && !json["neck"]["tiltForward"].isNull()) {
    			calNeckTiltForward = json["neck"]["tiltForward"].as<int16_t>();
    			Serial.printf("Calibrating Neck Tilt Forward: %d\n", calNeckTiltForward);
    		}
    		if (json["neck"].containsKey("tiltSideways") && !json["neck"]["tiltSideways"].isNull()) {
    			calNeckTiltSideways = json["neck"]["tiltSideways"].as<int16_t>();
    			Serial.printf("Calibrating Neck Tilt Sideways: %d\n", calNeckTiltSideways);
    		}
    	}

    	if (json.containsKey("body") && !json["body"].isNull()) {
    		if (json["body"].containsKey("rotation") && !json["body"]["rotation"].isNull()) {
    			calBodyRotation = json["body"]["rotation"].as<int16_t>();
    			Serial.printf("Calibrating Body Rotation: %d\n", calBodyRotation);
    		}
    		if (json["body"].containsKey("tiltForward") && !json["body"]["tiltForward"].isNull()) {
    			calBodyTiltForward = json["body"]["tiltForward"].as<int16_t>();
    			Serial.printf("Calibrating Body Tilt Forward: %d\n", calBodyTiltForward);
    		}
    		if (json["body"].containsKey("tiltSideways") && !json["body"]["tiltSideways"].isNull()) {
    			calBodyTiltSideways = json["body"]["tiltSideways"].as<int16_t>();
    			Serial.printf("Calibrating Body Tilt Sideways: %d\n", calBodyTiltSideways);
    		}
    	}

        if (json.containsKey("monocle") && !json["monocle"].isNull()) {
            if (json["monocle"].containsKey("position") && !json["monocle"]["position"].isNull()) {
                calMonoclePosition = json["monocle"]["position"].as<int16_t>();
                Serial.printf("Calibrating Monocle Position: %d\n", calMonoclePosition);
            }
        }

    	if (json.containsKey("action") && !json["action"].isNull()) {
    		String action = json["action"].as<String>();
    		if (action == "save") {
    			saveCalibration(); 
    		} else if (action == "reset") {
    			resetCalibrationToDefaults(); 
    		} else if (action == "set_middle_and_lock") {
                Serial.println("Action: Set Servos to Middle and Lock (Arduino side implementation needed!)");
                // *** IMPORTANT: You need to call your function here to physically move servos ***
                // Example: myRobot.setAllServosToMiddle();
            } else if (action == "unlock_servos") {
                Serial.println("Action: Unlock Servos (Arduino side implementation needed!)");
                // *** IMPORTANT: You need to call your function here to physically unlock servos ***
                // Example: myRobot.unlockAllServos();
            }
    	}

    	JsonDocument r; 
    	r["neck"]["rotation"] = calNeckRotation;
    	r["neck"]["tiltForward"] = calNeckTiltForward;
    	r["neck"]["tiltSideways"] = calNeckTiltSideways;
    	r["body"]["rotation"] = calBodyRotation;
    	r["body"]["tiltForward"] = calBodyTiltForward;
    	r["body"]["tiltSideways"] = calBodyTiltSideways;
        r["monocle"]["position"] = calMonoclePosition; 

    	String result;
    	serializeJson(r, result);
    	request->send(200, "application/json", result); 
    	Serial.println("apiCalibratePostAction response sent.");
    }

    void WebServer::apiGetCalibration(AsyncWebServerRequest *request) {
    	Serial.println("apiGetCalibration called.");
    	JsonDocument r; 
    	r["neck"]["rotation"] = calNeckRotation;
    	r["neck"]["tiltForward"] = calNeckTiltForward;
    	r["neck"]["tiltSideways"] = calNeckTiltSideways;
    	r["body"]["rotation"] = calBodyRotation;
    	r["body"]["tiltForward"] = calBodyTiltForward;
    	r["body"]["tiltSideways"] = calBodyTiltSideways;
        r["monocle"]["position"] = calMonoclePosition; 

    	String result;
    	serializeJson(r, result);
    	request->send(200, "application/json", result); 
    	Serial.println("apiGetCalibration response sent.");
    }

    void WebServer::apiLightsPostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    	Serial.println("apiLightsPostAction received.");

    	JsonDocument json;
    	DeserializationError error = deserializeJson(json, data, len); 
    	if (error) {
    		Serial.print(F("deserializeJson() failed: "));
    		Serial.println(error.f_str());
    		request->send(400, "text/plain", "Bad Request: Invalid JSON");
    		return;
    	}

    	if (json.containsKey("mode") && !json["mode"].isNull()) {
    		chestLightMode = (LightMode)json["mode"].as<uint8_t>(); 
    		Serial.printf("Chest Light Mode set to: %d (New Mode: %d)\n", chestLightMode, json["mode"].as<uint8_t>());
    	}

    	JsonDocument r; 
    	r["mode"] = (uint8_t)chestLightMode;
    	String result;
    	serializeJson(r, result);
    	request->send(200, "application/json", result); 
    	Serial.println("apiLightsPostAction response sent.");
    }

    String WebServer::getPage(Page page, AsyncWebServerRequest *request)
    {
    	String pageContent = "";
    	const char* filePath = "";

    	switch (page)
    	{
    	case WebServer::Page::indexPage: 
    		filePath = "/index.html";
    		break;
    	case WebServer::Page::settingsPage: 
    		filePath = "/settings.html";
    		break;
    	case WebServer::Page::calibrationPage: 
    		filePath = "/calibration.html";
    		break;
    	case WebServer::Page::lightsPage: 
    		filePath = "/chestlights.html";
    		break;
    	default:
    		Serial.println("WARNING: Unknown page requested in getPage!");
    		request->send(404, "text/plain", "Page Not Found"); 
    		return ""; 
    	}

    	pageContent = readFile(filePath);
    	if (pageContent.length() == 0) {
    		Serial.printf("ERROR: %s is empty or not found! Cannot serve page.\r\n", filePath);
    		request->send(500, "text/plain", "Server Error: Requested HTML file is missing or empty.");
    		return "";
    	}
    	
    	Serial.printf("Generated page for type %d (file: %s) with %d bytes.\n", page, filePath, pageContent.length());
    	return pageContent;
    }

    void WebServer::notFound(AsyncWebServerRequest *request)
    {
    	Serial.printf("404 Not Found: %s\n", request->url().c_str());
    	request->send(404, "text/plain", "Not found");
    }
    