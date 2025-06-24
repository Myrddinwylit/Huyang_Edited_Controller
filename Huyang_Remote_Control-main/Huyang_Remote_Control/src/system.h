#include "includes.h"    // This includes all necessary class headers like HuyangNeck.h
// #include "../config.h"   // REMOVED: This line caused redefinition errors as config.h is already included by the .ino
#include "../calibration.h" // Corrected path: calibration.h is in the parent directory (project root)
#include "definitions.h" // Corrected path: definitions.h is in the same src/ directory


void setup()
{
    Serial.begin(115200); // Used only for debugging on arduino serial monitor
    Serial.println("Huyang! v1.9 by Jeanette Müller");

    wifi->currentMode = WiFiDefaultMode;

    // Wifi Settings
    wifi->host = IPAddress(192, 168, 10, 1);
    wifi->subnetMask = IPAddress(255, 255, 255, 0);
    wifi->currentMode = JxWifiManager::WifiModeNetwork;

    // Hotspot
    wifi->hotspot_Ssid = WifiSsidOfHotspot;
    wifi->hotspot_Password = WifiPasswordHotspot;

    // use Local Wifi
    wifi->network_Ssid = WifiSsidConnectTo; // <- change if needed
    wifi->network_Password = WifiPasswordConnectTo;

    wifi->setup();

    // Pass the webserver pointer to the setup, which also loads calibration data
    webserver->setup(enableEyes,
                     enableMonacle,
                     enableNeckMovement,
                     enableHeadRotation,
                     enableBodyMovement,
                     enableBodyRotation,
                     enableTorsoLights);

    webserver->start(); // Start the web server and load calibration data

    pwm->begin();
    pwm->setOscillatorFrequency(27000000);
    pwm->setPWMFreq(60);

    huyangFace->setup();
    huyangBody->setup();
    huyangNeck->setup();
    // huyangAudio->setup(); // Audio setup commented out, keeping as is for now

    Serial.println("setup done");
}

void loop()
{
    currentMillis = millis();

    wifi->loop();
    webserver->loop(); // Crucial: Webserver loop to handle incoming requests and update state

    if (currentMillis - previousMillisIPAdress > 5000)
    {
        previousMillisIPAdress = currentMillis;
        Serial.print("Current IP: ");
        Serial.println(wifi->getCurrentIPAdress());
    }

    // Check if servos are locked for calibration. If so, do not apply manual controls.
    // This flag is managed by the WebServer in response to UI commands.
    if (!webserver->servosLockedForCalibration) {
        // Face control
        huyangFace->automatic = webserver->automaticAnimations; // Propagate automatic flag
        if (huyangFace->automatic == false) // If not automatic, apply manual eye commands from webserver
        {
            if (webserver->allEyes != 0) // If an 'all eyes' command is active
            {
                HuyangFace::EyeState newState = huyangFace->getStateFrom(webserver->allEyes);
                huyangFace->setEyesTo(newState);
                // If the command was Blink, reset it to Open after applying to allow for single blink
                if (newState == HuyangFace::EyeState::Blink)
                {
                    webserver->allEyes = HuyangFace::EyeState::Open;
                }
            }
            else // No 'all eyes' command, check individual eyes
            {
                if (webserver->leftEye != 0)
                {
                    HuyangFace::EyeState newState = huyangFace->getStateFrom(webserver->leftEye);
                    huyangFace->setLeftEyeTo(newState);
                    if (newState == HuyangFace::EyeState::Blink)
                    {
                        webserver->leftEye = HuyangFace::EyeState::Open;
                    }
                }

                if (webserver->rightEye != 0)
                {
                    HuyangFace::EyeState newState = huyangFace->getStateFrom(webserver->rightEye);
                    huyangFace->setRightEyeTo(newState);
                    if (newState == HuyangFace::EyeState::Blink)
                    {
                        webserver->rightEye = HuyangFace::EyeState::Open;
                    }
                }
            }
        }

        // Neck control
        huyangNeck->automatic = webserver->automaticAnimations; // Propagate automatic flag
        if (huyangNeck->automatic == false) // If not automatic, apply manual neck commands
        {
            // The calibration offset is now applied *inside* HuyangNeck if it uses them.
            huyangNeck->rotateHead(webserver->neckRotate);
            huyangNeck->tiltNeckForward(webserver->neckTiltForward);
            huyangNeck->tiltNeckSideways(webserver->neckTiltSideways);
            
            // Only set monocle position if monocle is enabled AND not in automatic mode
            if (enableMonacle) { // Check if monocle feature is enabled from config.h
                huyangNeck->setMonoclePosition(0); // Set monocle to hidden/default in manual mode
            }
        }

        // Body control
        huyangBody->automatic = webserver->automaticAnimations; // Propagate global automatic flag
        if (huyangBody->automatic == false) // Manual body control
        {
            // Pass the webserver values directly to body movement functions.
            // Calibration offsets are applied *internally* by HuyangBody using its stored _webserver pointer.
            huyangBody->rotateBody(webserver->bodyRotate);
            huyangBody->tiltBodyForward(webserver->bodyTiltForward);
            huyangBody->tiltBodySideways(webserver->bodyTiltSideways);
        }
    } // End if (!webserver->servosLockedForCalibration)

    // Always run the loop functions for each component, regardless of calibration lock or automatic mode.
    // Their internal logic decides whether to animate, hold position, or apply commands.
    huyangFace->loop();
    huyangNeck->loop();
    huyangBody->loop();

    // New: Torso Lights control
    // Only control if enabled in config.h (via the setup parameter)
    if (enableTorsoLights) {
        huyangBody->setTorsoLightMode(webserver->currentTorsoLightMode);
    }

    // huyangAudio->loop(); // Audio loop commented out, keep as is for now
}
