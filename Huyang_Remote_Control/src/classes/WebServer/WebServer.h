    #ifndef WEB_SERVER_H
    #define WEB_SERVER_H

    #include <ESPAsyncWebServer.h>
    #include <ArduinoJson.h>
    #include "FS.h" // For File System
    #include "LittleFS.h" // For LittleFS

    // Define light modes (updated with new modes)
    enum LightMode {
        LIGHT_OFF = 0,
        LIGHT_STATIC_BLUE = 1,          // Both LEDs static blue (initial mode)
        LIGHT_WARNING_BLINK = 2,        // Both blink alternatively Red/Blue
        LIGHT_PROCESSING_FADE = 3,      // Two LEDs fade randomly on/off, slow to fast fade, then slow blinks
        LIGHT_DROID_MODE_1 = 4,         // Star Wars Droid indicator lights - Mode 1
        LIGHT_DROID_MODE_2 = 5          // Star Wars Droid indicator lights - Mode 2
    };

    // --- GLOBAL VARIABLES DECLARATIONS (Accessible throughout your project) ---
    // These variables hold the current state of the robot.
    // They are updated by the WebServer and read by the HuyangRobot class (or similar).

    extern bool automaticAnimations; // true if robot is in automatic animation mode, false for manual control
    extern uint16_t allEyes;         // 0: no all-eye command, 1: open, 2: close, 3: blink, 4: focus, 5: sad, 6: angry
    extern uint16_t faceLeftEyeState;  // Current state of left eye (0: none, 1: open, 2: close, 3: blink, etc.)
    extern uint16_t faceRightEyeState; // Current state of right eye

    extern double neckRotate;      // Neck rotation value (-100 to 100)
    extern double neckTiltForward; // Neck tilt forward/back value (-100 to 100)
    extern double neckTiltSideways; // Neck tilt sideways value (-100 to 100)

    extern int16_t bodyRotate;      // Body rotation value (-100 to 100)
    extern int16_t bodyTiltForward; // Body tilt forward/back value (-100 to 100)
    extern int16_t bodyTiltSideways; // Body tilt sideways value (-100 to 100)

    extern int16_t monoclePosition; // Monocle servo position (e.g., 0-100, or mapped)

    // Calibration variables (used to adjust servo centers/ranges)
    extern int16_t calNeckRotation;
    extern int16_t calNeckTiltForward;
    extern int16_t calNeckTiltSideways;
    extern int16_t calBodyRotation;
    extern int16_t calBodyTiltForward;
    extern int16_t calBodyTiltSideways; // *** FIX: Added 'extern' here! ***
    extern int16_t calMonoclePosition; 

    extern LightMode chestLightMode; // Current mode for chest lights (now with more modes)

    class WebServer
    {
    public:
        // Enum for different HTML pages
        enum class Page {
            indexPage,
            settingsPage,
            calibrationPage,
            lightsPage // For chestlights.html
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

    private:
        AsyncWebServer *_server;

        // Feature enable flags (from config.h)
        bool _enableEyes;
        bool _enableMonacle;
        bool _enableNeckMovement;
        bool _enableHeadRotation;
        bool _enableBodyMovement;
        bool _enableBodyRotation;
        bool _enableTorsoLights;

        // Helper functions for file operations
        String readFile(const char *path);
        void writeFile(const char *path, const char *message);

        // Calibration management functions
        void loadCalibration();
        void saveCalibration();
        void resetCalibrationToDefaults();

        // API action handlers
        void apiPostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
        void apiCalibratePostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
        void apiLightsPostAction(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
        void apiGetCalibration(AsyncWebServerRequest *request);

        // HTML page serving function
        String getPage(Page page, AsyncWebServerRequest *request);
        void notFound(AsyncWebServerRequest *request);
    };

    #endif
    