// This file contains the setup() and loop() functions for the main Arduino program.

// Include necessary headers. Note: The definitions for global variables/objects
// are now exclusively in Huyang_Remote_Control.ino.
#include "includes.h"    // includes.h is in the same 'src' directory
#include "../config.h"         // config.h is in the parent directory
#include "../calibration.h"    // calibration.h is in the parent directory
#include "definitions.h" // definitions.h is in the same 'src' directory

void setup()
{
    Serial.begin(115200); // Start serial communication for debugging
    Serial.println("Huyang! v1.9 by Jeanette Müller");

    // Wi-Fi setup based on configuration from config.h
    wifi->currentMode = WiFiDefaultMode;
    wifi->host = IPAddress(192, 168, 10, 1); // Default AP IP
    wifi->subnetMask = IPAddress(255, 255, 255, 0);
    wifi->hotspot_Ssid = WifiSsidOfHotspot;
    wifi->hotspot_Password = WifiPasswordHotspot;
    wifi->network_Ssid = WifiSsidConnectTo;
    wifi->network_Password = WifiPasswordConnectTo;
    wifi->setup(); // Initialize Wi-Fi connection

    // Web server setup: pass feature enable flags from config.h
    webserver->setup(enableEyes,
                     enableMonacle,
                     enableNeckMovement,
                     enableHeadRotation,
                     enableBodyMovement,
                     enableBodyRotation,
                     enableTorsoLights);
    webserver->start(); // Start the web server

    // PWM driver (PCA9685) setup
    pwm->begin();
    pwm->setOscillatorFrequency(27000000); // Set oscillator frequency
    pwm->setPWMFreq(60); // Set PWM frequency for servos

    // Robot subsystem setup
    huyangFace->setup(); // Setup eye displays
    huyangBody->setup(); // Setup body servos and chest lights
    huyangNeck->setup(); // Setup neck servos
    // huyangAudio->setup(); // Audio setup (currently commented out)

    Serial.println("Setup done!");
}

void loop()
{
    currentMillis = millis(); // Update current time

    wifi->loop(); // Wi-Fi manager loop

    // Periodically print IP address to serial monitor
    if (currentMillis - previousMillisIPAdress > 5000)
    {
        previousMillisIPAdress = currentMillis;

        if (wifi->isConnected())
        {
            IPAddress currentAdress = wifi->getCurrentIPAdress();
            Serial.print("Open Huyang Webinterface via Browser by opening http://");
            Serial.print(currentAdress);
            if (WebServerPort != 80)
            {
                Serial.print(":");
                Serial.print(WebServerPort);
            }
            Serial.println("/"); // Add a new line for cleaner output
        } else {
             Serial.println("WiFi not connected. Attempting to connect or create hotspot...");
        }
    }

    // --- Control Face (Eyes) ---
    // Access automaticAnimations directly as it's a global extern variable
    huyangFace->automatic = automaticAnimations; 

    if (automaticAnimations == false) // If manual control (access directly)
    {
        // Access allEyes directly as it's a global extern variable
        if (allEyes != 0) // If an "all eyes" command was sent
        {
            HuyangFace::EyeState newState = huyangFace->getStateFrom(allEyes); // Access directly
            huyangFace->setEyesTo(newState);
            if (newState == HuyangFace::EyeState::Blink)
            {
                allEyes = HuyangFace::EyeState::Open; // Reset blink to open after one blink cycle (access directly)
            }
        }
        else // If individual eye commands were sent
        {
            // Access faceLeftEyeState and faceRightEyeState directly
            if (faceLeftEyeState != 0) 
            {
                HuyangFace::EyeState newState = huyangFace->getStateFrom(faceLeftEyeState); 
                huyangFace->setLeftEyeTo(newState);
                if (newState == HuyangFace::EyeState::Blink)
                {
                    faceLeftEyeState = HuyangFace::EyeState::Open; 
                }
            }
            if (faceRightEyeState != 0) 
            {
                HuyangFace::EyeState newState = huyangFace->getStateFrom(faceRightEyeState); 
                huyangFace->setRightEyeTo(newState);
                if (newState == HuyangFace::EyeState::Blink)
                {
                    faceRightEyeState = HuyangFace::EyeState::Open; 
                }
            }
        }
    }
    huyangFace->loop(); // Run the face control loop

    // --- Control Neck ---
    // Access automaticAnimations directly
    huyangNeck->automatic = automaticAnimations; 

    if (automaticAnimations == false) // If manual control
    {
        // Access neckRotate, calNeckRotation, etc., directly as global extern variables
        double calibratedNeckRotate = neckRotate + calNeckRotation;
        double calibratedNeckTiltForward = neckTiltForward + calNeckTiltForward;
        double calibratedNeckTiltSideways = neckTiltSideways + calNeckTiltSideways;

        huyangNeck->rotateHead(calibratedNeckRotate);
        huyangNeck->tiltNeckForward(calibratedNeckTiltForward);
        huyangNeck->tiltNeckSideways(calibratedNeckTiltSideways);
    }
    huyangNeck->loop(); // Run the neck control loop

    // --- Control Body ---
    // Access automaticAnimations directly
    huyangBody->automatic = automaticAnimations; 

    if (automaticAnimations == false) // If manual control
    {
        // Access bodyRotate, calBodyRotation, etc., directly as global extern variables
        int16_t calibratedBodyRotate = bodyRotate + calBodyRotation;
        int16_t calibratedBodyTiltForward = bodyTiltForward + calBodyTiltForward;
        int16_t calibratedBodyTiltSideways = bodyTiltSideways + calBodyTiltSideways;

        huyangBody->rotateBody(calibratedBodyRotate);
        huyangBody->tiltBodyForward(calibratedBodyTiltForward);
        huyangBody->tiltBodySideways(calibratedBodyTiltSideways);
    }
    // Access chestLightMode directly
    huyangBody->currentLightMode = (HuyangBody::LightMode)chestLightMode;

    huyangBody->loop(); // Run the body control loop

    // huyangAudio->loop(); // Audio loop (currently commented out)
}
