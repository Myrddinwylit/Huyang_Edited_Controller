// Global variables for managing robot state
var automatic = true;

var face_eyes_all = null;
var face_eyes_left = 0; 
var face_eyes_right = 0; 

var neck_rotate = 0;
var neck_tiltForward = 0;
var neck_tiltSideways = 0;

var body_rotate = 0;
var body_tiltForward = 0;
var body_tiltSideways = 0;

var monoclePosition = 0; // Global variable for monocle current position

var JoyNeck; 
var JoyNeckX = 0;
var JoyNeckY = 0;

var JoyBody; 
var JoyBodyX = 0;
var JoyBodyY = 0;

// Chest light mode (reflects enum in WebServer.h)
// 0: OFF, 1: STATIC_BLUE, 2: WARNING_BLINK, 3: PROCESSING_FADE, 4: DROID_MODE_1, 5: DROID_MODE_2
var chestLightMode = 1; // Default to STATIC_BLUE

// --- NEW Global variables for Settings ---
var robotName = "Huyang Robot";
var masterMovementSpeed = 100; // Percentage 50-150

// --- Global variables for LED visual animations ---
let currentSetIntervalAnimationId = null; // For setInterval-based animations
let currentRequestAnimationFrameAnimationId = null; // For requestAnimationFrame-based animations

const LED_COLORS = { // Define common colors for LEDs
    BLUE: 'rgb(0, 0, 255)',
    RED: 'rgb(255, 0, 0)',
    GREEN: 'rgb(0, 255, 0)',
    MAGENTA: 'rgb(255, 0, 255)',
    YELLOW: 'rgb(255, 255, 0)',
    OFF: 'rgb(51, 51, 51)' // Dark gray for off state
};


function sendEyeUpdate(position, action) {
    let data = {
        automatic: false,
        face: {
            left: face_eyes_left, 
            right: face_eyes_right
        }
    };

    if (position === 'all') {
        data.face.all = action; 
        face_eyes_left = action; 
        face_eyes_right = action; 
    } else {
        data.face[position] = action;
        if (position === 'left') face_eyes_left = action;
        if (position === 'right') face_eyes_right = action;
    }

    console.log(`sendEyeUpdate: Sending data for ${position} eye:`, data);
    sendData(data);
}

function sendMonocleUpdate(position) {
    const data = {
        automatic: false,
        face: {
            monocle: position 
        }
    };
    console.log("sendMonocleUpdate: Sending monocle data:", data);
    sendData(data);
}

function changeAutomatic(newState) {
    const data = {
        automatic: newState
    };
    sendData(data);
}

function sendNeckUpdate() {
    if (JoyNeck) { 
        const data = {
            automatic: false,
            neck: {
                rotate: parseInt(JoyNeck.GetX()), 
                tiltForward: parseInt(JoyNeck.GetY()), 
                tiltSideways: document.getElementById('slider_neckTiltSideways') ? parseInt(document.getElementById('slider_neckTiltSideways').value) : neck_tiltSideways 
            }
        };
        console.log("sendNeckUpdate: Sending neck data:", data);
        sendData(data);
    } else {
        console.warn("sendNeckUpdate: JoyNeck not initialized or element not found.");
    }
}

function sendBodyUpdate() {
    if (JoyBody) { 
        const data = {
            automatic: false,
            body: {
                rotate: parseInt(JoyBody.GetX()), 
                tiltForward: parseInt(JoyBody.GetY()), 
                tiltSideways: document.getElementById('slider_bodyTiltSideways') ? parseInt(document.getElementById('slider_bodyTiltSideways').value) : body_tiltSideways 
            }
        };
        console.log("sendBodyUpdate: Sending body data:", data);
        sendData(data);
    } else {
        console.warn("sendBodyUpdate: JoyBody not initialized or element not found.");
    }
}

function sendChestLightUpdate(mode) {
    const data = {
        mode: mode 
    };
    console.log(`sendChestLightUpdate: Sending light mode: ${mode}`);
    postDataJson('api/lights.json', data).then(json => {
        console.log('Result from Lights Server:', json);
        if (json.mode != null) {
            chestLightMode = json.mode;
        }
        updateUserInterface(); // Update UI to reflect the new state and visual LEDs
    }).catch(error => {
        console.error("Error sending light data or parsing response:", error);
    });
}

// --- NEW: Functions for Settings Page ---
async function initSettings() {
    console.log("initSettings: Attempting to fetch settings data.");
    try {
        const response = await fetch("/api/settings/get.json"); // New API endpoint for settings
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        console.log("initSettings: Received settings data:", data);

        // Populate robot name
        const robotNameInput = document.getElementById('robot_name_input');
        if (robotNameInput && data.robotName != null) {
            robotName = data.robotName;
            robotNameInput.value = robotName;
        }

        // Populate master movement speed
        const masterSpeedSlider = document.getElementById('master_speed_slider');
        const masterSpeedValueDisplay = document.getElementById('master_speed_value');
        if (masterSpeedSlider && masterSpeedValueDisplay && data.masterSpeed != null) {
            masterMovementSpeed = data.masterSpeed;
            masterSpeedSlider.value = masterMovementSpeed;
            masterSpeedValueDisplay.innerText = masterMovementSpeed + '%';
        }

        // Populate firmware version (assuming it's returned here)
        const firmwareVersionDisplay = document.getElementById('firmware_version_display');
        if (firmwareVersionDisplay && data.firmwareVersion != null) {
            firmwareVersionDisplay.innerText = data.firmwareVersion;
        }

    } catch (error) {
        console.error("initSettings: Failed to fetch settings data:", error);
        const firmwareVersionDisplay = document.getElementById('firmware_version_display');
        if (firmwareVersionDisplay) firmwareVersionDisplay.innerText = "Error loading";
    }
}

// Send settings updates to the ESP32
async function sendSettingsUpdate() {
    console.log("sendSettingsUpdate: Preparing to send settings data.");
    const robotNameInput = document.getElementById('robot_name_input');
    const masterSpeedSlider = document.getElementById('master_speed_slider');

    let data = {};
    if (robotNameInput) {
        data.robotName = robotNameInput.value;
        robotName = robotNameInput.value; // Update client-side variable immediately
    }
    if (masterSpeedSlider) {
        data.masterSpeed = parseInt(masterSpeedSlider.value);
        masterMovementSpeed = parseInt(masterSpeedSlider.value); // Update client-side variable
    }

    console.log("sendSettingsUpdate: Sending data:", data);
    try {
        await postDataJson("/api/settings.json", data); // New API endpoint for settings POST
        console.log("sendSettingsUpdate: Settings updated successfully.");
    } catch (error) {
        console.error("sendSettingsUpdate: Failed to send settings data:", error);
    }
}

// Function to confirm and send reboot command
function confirmReboot() {
    // Custom modal confirmation, not alert()
    // For now, let's use a simple confirm as a placeholder for quick testing
    if (confirm("Are you sure you want to reboot the robot? This will temporarily disconnect the web interface.")) {
        sendRebootCommand();
    }
}

async function sendRebootCommand() {
    console.log("sendRebootCommand: Sending reboot command.");
    try {
        await postDataJson("/api/system.json", { action: "reboot" }); // New API endpoint for system actions
        console.log("Reboot command sent. Robot should restart shortly.");
        // You might want to display a message to the user that the robot is rebooting
        // and then try to refresh the page after a delay.
        setTimeout(() => { window.location.reload(); }, 5000); 
    } catch (error) {
        console.error("sendRebootCommand: Failed to send reboot command:", error);
        alert("Failed to send reboot command. Check robot connection.");
    }
}

// Function to confirm and send factory reset command
function confirmFactoryReset() {
    // Custom modal confirmation, not alert()
    if (confirm("WARNING: Are you absolutely sure you want to perform a Factory Reset?\n\nTHIS WILL ERASE ALL SAVED SETTINGS AND CALIBRATION DATA and cannot be undone!")) {
        if (confirm("Are you REALLY sure? This is your last chance to cancel.")) {
            sendFactoryResetCommand();
        }
    }
}

async function sendFactoryResetCommand() {
    console.log("sendFactoryResetCommand: Sending factory reset command.");
    try {
        await postDataJson("/api/system.json", { action: "factory_reset" }); // System endpoint
        console.log("Factory reset command sent. Robot will reboot with default settings.");
        setTimeout(() => { window.location.reload(); }, 5000); 
    } catch (error) {
        console.error("sendFactoryResetCommand: Failed to send factory reset command:", error);
        alert("Failed to send factory reset command. Check robot connection.");
    }
}


async function postDataJson(url = "", data = {}) {
    try {
        const response = await fetch(url, {
            method: 'POST',
            mode: 'cors', 
            cache: 'no-cache',
            headers: {
                'Content-Type': 'application/json'
            },
            referrerPolicy: 'no-referrer', 
            body: JSON.stringify(data),
        });
        if (!response.ok) {
            const errorText = await response.text();
            throw new Error(`HTTP error! Status: ${response.status}, Body: ${errorText}`);
        }
        return await response.json();
    } catch (error) {
        console.error("Fetch/JSON parsing error in postDataJson:", error);
        throw error; 
    }
}

function sendData(data) {
    console.log("sendData: Preparing to send data to /api/post.json:", data);

    postDataJson('/api/post.json', data).then(json => {
        console.log('sendData: Result from Server (/api/post.json):', json);

        if (json.automatic != null) {
            automatic = json.automatic;
        }

        if (json.face) { 
            if (json.face.eyes != null) { 
                face_eyes_all = json.face.eyes.all != null ? json.face.eyes.all : face_eyes_all;
                face_eyes_left = json.face.eyes.left != null ? json.face.eyes.left : face_eyes_left;
                face_eyes_right = json.face.eyes.right != null ? json.face.eyes.right : face_eyes_right;
            }
            if (json.face.monocle != null && json.face.monocle.position != null) {
                monoclePosition = json.face.monocle.position;
            }
        }

        if (json.neck != null) {
            neck_rotate = json.neck.rotate != null ? json.neck.rotate : neck_rotate;
            neck_tiltForward = json.neck.tiltForward != null ? json.neck.tiltForward : neck_tiltForward;
            neck_tiltSideways = json.neck.tiltSideways != null ? json.neck.tiltSideways : neck_tiltSideways;
        }

        if (json.body != null) {
            body_rotate = json.body.rotate != null ? json.body.rotate : body_rotate;
            body_tiltForward = json.body.tiltForward != null ? json.body.tiltForward : body_tiltForward;
            body_tiltSideways = json.body.tiltSideways != null ? json.body.tiltSideways : body_tiltSideways;
        }
        if (json.chestLightMode != null) { 
            chestLightMode = json.chestLightMode;
        }
        // NEW: Update robotName and masterMovementSpeed if received in general /api/post.json
        if (json.robotName != null) {
            robotName = json.robotName;
        }
        if (json.masterMovementSpeed != null) {
            masterMovementSpeed = json.masterMovementSpeed;
        }

        console.log("sendData: Calling updateUserInterface()...");
        updateUserInterface(); 
    }).catch(error => {
        console.error("sendData: Error in data sending/processing chain:", error); 
    });
}

function updateUserInterface() {
    console.log("updateUserInterface: Updating UI elements.");
    const button_automatic = document.getElementById('button_automatic');
    if (button_automatic) {
        button_automatic.innerHTML = automatic ? "Auto is ON" : "Auto is OFF";
        if (automatic) {
            button_automatic.classList.add('selected');
        } else {
            button_automatic.classList.remove('selected');
        }
    } 

    const eyeControlElements = [
        { id: 'eye_all_open', state: 1, type: 'all' }, { id: 'eye_all_close', state: 2, type: 'all' }, 
        { id: 'eye_all_blink', state: 3, type: 'all' }, { id: 'eye_all_focus', state: 4, type: 'all' }, 
        { id: 'eye_all_sad', state: 5, type: 'all' }, { id: 'eye_all_angry', state: 6, type: 'all' },
        { id: 'eye_left_open', state: 1, type: 'left' }, { id: 'eye_left_close', state: 2, type: 'left' }, 
        { id: 'eye_left_blink', state: 3, type: 'left' }, { id: 'eye_left_focus', state: 4, type: 'left' }, 
        { id: 'eye_left_sad', state: 5, type: 'left' }, { id: 'eye_left_angry', state: 6, type: 'left' },
        { id: 'eye_right_open', state: 1, type: 'right' }, { id: 'eye_right_close', state: 2, type: 'right' }, 
        { id: 'eye_right_blink', state: 3, type: 'right' }, { id: 'eye_right_focus', state: 4, type: 'right' }, 
        { id: 'eye_right_sad', state: 5, type: 'right' }, { id: 'eye_right_angry', state: 6, type: 'right' }
    ];

    const activeStateClass = 'selected';

    eyeControlElements.forEach(item => {
        const btn = document.getElementById(item.id);
        if (btn) {
            btn.classList.remove(activeStateClass); 

            let shouldBeSelected = false;
            if (item.type === 'all' && face_eyes_all !== null) { 
                shouldBeSelected = (face_eyes_all === item.state);
            } else if (item.type === 'left' && face_eyes_left !== null) {
                shouldBeSelected = (face_eyes_left === item.state);
            } else if (item.type === 'right' && face_eyes_right !== null) {
                shouldBeSelected = (face_eyes_right === item.state);
            }
            
            if (shouldBeSelected) {
                btn.classList.add(activeStateClass); 
            }
        }
    });
    console.log("updateUserInterface: Eye buttons processed.");

    const slider_neckTiltSideways = document.getElementById('slider_neckTiltSideways');
    if (slider_neckTiltSideways) {
        slider_neckTiltSideways.value = neck_tiltSideways;
    } 
    
    const slider_bodyTiltSideways = document.getElementById('slider_bodyTiltSideways');
    if (slider_bodyTiltSideways) {
        slider_bodyTiltSideways.value = body_tiltSideways;
    } 

    const monoclePositionSliderIndex = document.getElementById('slider_monoclePosition'); 
    if (monoclePositionSliderIndex) {
        monoclePositionSliderIndex.value = monoclePosition; 
        console.log('updateUserInterface: Index Page Monocle Position slider value set to:', monoclePosition);
    } 

    // Light buttons now handle all 6 modes
    const lightButtonsMap = {
        'button_chestLightOff': 0,
        'button_chestLightStatic': 1,
        'button_chestLightWarning': 2,
        'button_chestLightProcessing': 3,
        'button_chestLightDroid1': 4,
        'button_chestLightDroid2': 5
    };

    Object.keys(lightButtonsMap).forEach(id => {
        const button = document.getElementById(id);
        if (button) {
            button.classList.toggle(activeStateClass, chestLightMode === lightButtonsMap[id]);
        }
    });
    console.log("updateUserInterface: Light buttons processed.");
    
    // --- Call the new LED visualizer function here (only if LEDs are present) ---
    if (document.getElementById('visual_led_0') && document.getElementById('visual_led_1')) {
        updateLedVisuals();
    }
}

// --- New function to update LED visualizers based on chestLightMode ---
function updateLedVisuals() {
    // Clear any existing animation intervals/frames
    if (currentSetIntervalAnimationId) {
        clearInterval(currentSetIntervalAnimationId);
        currentSetIntervalAnimationId = null;
    }
    if (currentRequestAnimationFrameAnimationId) {
        cancelAnimationFrame(currentRequestAnimationFrameAnimationId);
        currentRequestAnimationFrameAnimationId = null;
    }

    const led0 = document.getElementById('visual_led_0');
    const led1 = document.getElementById('visual_led_1');
    const LED_ELEMENTS = [led0, led1].filter(Boolean); // Filter out nulls if an element isn't found

    if (LED_ELEMENTS.length === 0) {
        console.warn("LED visual elements not found in DOM. Skipping visual updates for LEDs.");
        return;
    }

    // Reset LEDs to default off state before applying new mode
    LED_ELEMENTS.forEach(led => {
        led.style.backgroundColor = LED_COLORS.OFF;
        led.classList.remove('active-glow');
        led.style.filter = 'brightness(100%)'; // Reset brightness filter
        led.style.transition = 'background-color 0.1s ease-in-out, box-shadow 0.1s ease-in-out, filter 0.1s ease-in-out'; // Ensure transitions for smooth changes
    });

    console.log(`updateLedVisuals: Setting LEDs for mode: ${chestLightMode}`);

    switch (chestLightMode) {
        case 0: // LIGHT_OFF
            // Already handled by initial reset
            break;

        case 1: // LIGHT_STATIC_BLUE
            LED_ELEMENTS.forEach(led => {
                led.style.backgroundColor = LED_COLORS.BLUE;
                led.classList.add('active-glow');
                led.style.color = LED_COLORS.BLUE; // For glow effect
                led.style.filter = 'brightness(100%)';
            });
            break;

        case 2: // LIGHT_WARNING_BLINK (Red/Blue Alternating)
            let warnBlinkState = false;
            currentSetIntervalAnimationId = setInterval(() => {
                warnBlinkState = !warnBlinkState;
                if (warnBlinkState) {
                    led0.style.backgroundColor = LED_COLORS.RED;
                    led0.classList.add('active-glow');
                    led0.style.color = LED_COLORS.RED;
                    led1.style.backgroundColor = LED_COLORS.BLUE;
                    led1.classList.add('active-glow');
                    led1.style.color = LED_COLORS.BLUE;
                } else {
                    led0.style.backgroundColor = LED_COLORS.BLUE;
                    led0.classList.add('active-glow');
                    led0.style.color = LED_COLORS.BLUE;
                    led1.style.backgroundColor = LED_COLORS.RED;
                    led1.classList.add('active-glow');
                    led1.style.color = LED_COLORS.RED; 
                }
                LED_ELEMENTS.forEach(led => led.style.filter = 'brightness(100%)'); // Ensure full brightness
            }, 500); // Blink every 500ms (1 second cycle for full alternation)
            break;

        case 3: // LIGHT_PROCESSING_FADE (Slow to Fast Fade then Slow Blinks)
            let processingStartTime = null;
            let phase = 0; // 0: initial slow fade, 1: faster fade, 2: intermittent blinks

            function processingAnimation(timestamp) {
                if (!processingStartTime) processingStartTime = timestamp;
                const elapsed = timestamp - processingStartTime;

                if (phase === 0) { // Initial slow fade (e.g., first 10 seconds of animation cycle)
                    if (elapsed < 10000) {
                        const normalizedTime = (elapsed % 4000) / 4000; // Slower breath cycle (4 seconds)
                        const brightness = 0.5 + 0.5 * Math.sin(normalizedTime * Math.PI * 2); 
                        LED_ELEMENTS.forEach(led => {
                            led.style.backgroundColor = LED_COLORS.BLUE;
                            led.style.filter = `brightness(${Math.max(brightness, 0.2) * 100}%)`; // Min brightness 20%
                            led.classList.add('active-glow');
                            led.style.color = LED_COLORS.BLUE;
                        });
                    } else {
                        phase = 1; // Transition to faster fade
                        processingStartTime = timestamp; // Reset start time for new phase
                        console.log("Processing Mode: Transitioned to faster fade.");
                    }
                } else if (phase === 1) { // Faster fade (e.g., next 10 seconds)
                    if (elapsed < 10000) {
                        const normalizedTime = (elapsed % 1500) / 1500; // Faster breath cycle (1.5 seconds)
                        const brightness = 0.5 + 0.5 * Math.sin(normalizedTime * Math.PI * 2);
                        LED_ELEMENTS.forEach(led => {
                            led.style.backgroundColor = LED_COLORS.BLUE;
                            led.style.filter = `brightness(${Math.max(brightness, 0.2) * 100}%)`;
                            led.classList.add('active-glow');
                            led.style.color = LED_COLORS.BLUE;
                        });
                    } else {
                        phase = 2; // Transition to intermittent blinks
                        processingStartTime = timestamp; // Reset start time for new phase
                        console.log("Processing Mode: Transitioned to intermittent blinks.");
                        // Ensure LEDs are at full brightness before blinking starts
                        LED_ELEMENTS.forEach(led => {
                            led.style.filter = 'brightness(100%)';
                            led.style.backgroundColor = LED_COLORS.BLUE;
                            led.classList.add('active-glow');
                            led.style.color = LED_COLORS.BLUE;
                        });
                    }
                } else if (phase === 2) { // Intermittent slow blinks (random timings)
                    const blinkOnDuration = 200; // How long it's on
                    const blinkOffMin = 1000;
                    const blinkOffMax = 3000; // Random off time between 1s and 3s
                    
                    // Per-LED random off timings for more organic feel
                    if (!led0._nextBlinkTime) led0._nextBlinkTime = timestamp + blinkOnDuration + blinkOffMin + Math.random() * (blinkOffMax - blinkOffMin);
                    if (!led1._nextBlinkTime) led1._nextBlinkTime = timestamp + blinkOnDuration + blinkOffMin + Math.random() * (blinkOffMax - blinkOffMin);

                    if (timestamp < led0._nextBlinkTime) {
                        led0.style.backgroundColor = LED_COLORS.BLUE;
                        led0.style.filter = 'brightness(100%)';
                        led0.classList.add('active-glow');
                        led0.style.color = LED_COLORS.BLUE;
                    } else {
                        led0.style.backgroundColor = LED_COLORS.OFF;
                        led0.classList.remove('active-glow');
                        if (timestamp > led0._nextBlinkTime + blinkOnDuration) { // If it's been off for long enough, reset for next blink
                            led0._nextBlinkTime = timestamp + blinkOnDuration + blinkOffMin + Math.random() * (blinkOffMax - blinkOffMin);
                        }
                    }

                    if (timestamp < led1._nextBlinkTime) {
                        led1.style.backgroundColor = LED_COLORS.BLUE;
                        led1.style.filter = 'brightness(100%)';
                        led1.classList.add('active-glow');
                        led1.style.color = LED_COLORS.BLUE;
                    } else {
                        led1.style.backgroundColor = LED_COLORS.OFF;
                        led1.classList.remove('active-glow');
                        if (timestamp > led1._nextBlinkTime + blinkOnDuration) {
                            led1._nextBlinkTime = timestamp + blinkOnDuration + blinkOffMin + Math.random() * (blinkOffMax - blinkOffMin);
                        }
                    }
                }
                currentRequestAnimationFrameAnimationId = requestAnimationFrame(processingAnimation);
            }
            currentRequestAnimationFrameAnimationId = requestAnimationFrame(processingAnimation);
            break;


        case 4: // LIGHT_DROID_MODE_1 (Sequential Green)
            let currentLedIndex = 0;
            currentSetIntervalAnimationId = setInterval(() => {
                LED_ELEMENTS.forEach((led, index) => {
                    if (index === currentLedIndex) {
                        led.style.backgroundColor = LED_COLORS.GREEN;
                        led.classList.add('active-glow');
                        led.style.color = LED_COLORS.GREEN;
                    } else {
                        led.style.backgroundColor = LED_COLORS.OFF;
                        led.classList.remove('active-glow');
                    }
                    led.style.filter = 'brightness(100%)';
                });
                currentLedIndex = (currentLedIndex + 1) % LED_ELEMENTS.length;
            }, 150); 
            break;

        case 5: // LIGHT_DROID_MODE_2 (Random Colors, Both LEDs)
            currentSetIntervalAnimationId = setInterval(() => {
                LED_ELEMENTS.forEach(led => {
                    const randomColor = `rgb(${Math.random() * 255}, ${Math.random() * 255}, ${Math.random() * 255})`;
                    led.style.backgroundColor = randomColor;
                    led.classList.add('active-glow');
                    led.style.color = randomColor; // For glow
                    led.style.filter = 'brightness(100%)';
                });
            }, 200); 
            break;
    }
}


async function getServerData() {
    console.log('getServerData: Fetching initial server data.');
    await postDataJson("/api/post.json", {}); // Use postDataJson with an empty object for GET
}

function systemInit() {
    console.log('systemInit: Script execution started for index.html.');
    getServerData(); 
    initJoystick(); 
    setInterval(getServerData, 2000); 
}

async function sendCalibrationUpdate(component, axis, value) {
    console.log(`sendCalibrationUpdate: Updating ${component} ${axis} to ${value}`);
    let data = {};
    if (!data[component]) {
        data[component] = {};
    }
    data[component][axis] = parseInt(value); 

    await postDataJson("/api/calibrate.json", data); 
    initCalibrationSliders();
}

async function sendCalibrationSave() {
    console.log("sendCalibrationSave: Saving calibration data.");
    await postDataJson("/api/calibrate.json", { action: "save" }); 
}

async function sendCalibrationReset() {
    console.log("sendCalibrationReset: Resetting calibration data.");
    await postDataJson("/api/calibrate.json", { action: "reset" }); 
    initCalibrationSliders();
}

async function sendCalibrationSetMiddleAndLock() {
    console.log("sendCalibrationSetMiddleAndLock: Setting middle and locking servos.");
    await postDataJson("/api/calibrate.json", { action: "set_middle_and_lock" }); 
}

async function sendCalibrationUnlockServos() {
    console.log("sendCalibrationUnlockServos: Unlocking servos.");
    await postDataJson("/api/calibrate.json", { action: "unlock_servos" }); 
}

async function initCalibrationSliders() {
    console.log("initCalibrationSliders: Attempting to fetch calibration data.");
    try {
        const response = await fetch("/api/calibration/get.json");
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        console.log("initCalibrationSliders: Received calibration data:", data);

        // Update client-side variables and slider positions
        // Ensure IDs match the calibration.html
        const calNeckRotationElem = document.getElementById('cal_neck_rotation');
        if (calNeckRotationElem && data.neck) {
            calNeckRotationElem.value = data.neck.rotation || 0;
            document.getElementById('cal_neck_rotation_value').innerText = data.neck.rotation || 0;
        }
        const calNeckTiltForwardElem = document.getElementById('cal_neck_tilt_forward');
        if (calNeckTiltForwardElem && data.neck) {
            calNeckTiltForwardElem.value = data.neck.tiltForward || 0;
            document.getElementById('cal_neck_tilt_forward_value').innerText = data.neck.tiltForward || 0;
        }
        const calNeckTiltSidewaysElem = document.getElementById('cal_neck_tilt_sideways');
        if (calNeckTiltSidewaysElem && data.neck) {
            calNeckTiltSidewaysElem.value = data.neck.tiltSideways || 0;
            document.getElementById('cal_neck_tilt_sideways_value').innerText = data.neck.tiltSideways || 0;
        }

        const calBodyRotationElem = document.getElementById('cal_body_rotation');
        if (calBodyRotationElem && data.body) {
            calBodyRotationElem.value = data.body.rotation || 0;
            document.getElementById('cal_body_rotation_value').innerText = data.body.rotation || 0;
        }
        const calBodyTiltForwardElem = document.getElementById('cal_body_tilt_forward');
        if (calBodyTiltForwardElem && data.body) {
            calBodyTiltForwardElem.value = data.body.tiltForward || 0;
            document.getElementById('cal_body_tilt_forward_value').innerText = data.body.tiltForward || 0;
        }
        const calBodyTiltSidewaysElem = document.getElementById('slider_bodyTiltSideways'); // This ID is also on index.html
        if (calBodyTiltSidewaysElem && data.body) {
            calBodyTiltSidewaysElem.value = data.body.tiltSideways || 0;
            document.getElementById('cal_body_tilt_sideways_value').innerText = data.body.tiltSideways || 0;
        }

        const calMonoclePositionElem = document.getElementById('cal_monocle_position');
        if (calMonoclePositionElem && data.monocle) {
            calMonoclePositionElem.value = data.monocle.position || 0;
            document.getElementById('cal_monocle_position_value').innerText = data.monocle.position || 0;
        } else {
             console.log("initCalibrationSliders: Monocle position slider not found or data missing (expected if on index.html).");
        }


    } catch (error) {
        console.error("initCalibrationSliders: Failed to fetch calibration data:", error);
    }
}


function initJoystick() {
    console.log("initJoystick: Called.");

    const fillColor = '#00ff99'; 
    const strokeColor = '#00ffff'; 

    const joystickSettings = {
        autoReturnToCenter: true,
        internalFillColor: fillColor,
        internalLineWidth: 2,
        internalStrokeColor: strokeColor,
        externalLineWidth: 2,
        externalStrokeColor: strokeColor
    };

    const joyNeckElement = document.getElementById('joyNeck');
    if (typeof JoyStick !== 'undefined' && JoyNeck == null && joyNeckElement != null) {
        console.log("initJoystick: Initializing joyNeck.");
        try {
            JoyNeck = new JoyStick('joyNeck', joystickSettings, function (stickData) {
                if (JoyNeckX !== stickData.x || JoyNeckY !== stickData.y) {
                    JoyNeckX = stickData.x;
                    JoyNeckY = stickData.y;
                    sendNeckUpdate(); 
                }
            });
            console.log("initJoystick: JoyNeck initialized successfully:", JoyNeck);
        } catch (e) {
            console.error("initJoystick: Error initializing JoyNeck:", e);
        }
    } else {
        // console.log("initJoystick: joyNeck canvas ele/Users/brmark/Desktop/Huyang_Remote_Control-main/Huyang_Remote_Control/data/styles.cssment NOT found in DOM. (Expected if not on index.html)"); 
    }

    const joyBodyElement = document.getElementById('joyBody');
    if (typeof JoyStick !== 'undefined' && JoyBody == null && joyBodyElement != null) {
        console.log("initJoystick: Initializing joyBody.");
        try {
            JoyBody = new JoyStick('joyBody', joystickSettings, function (stickData) {
                if (JoyBodyX !== stickData.x || JoyBodyY !== stickData.y) {
                    JoyBodyX = stickData.x;
                    JoyBodyY = stickData.y;
                    sendBodyUpdate(); 
                }
            });
            console.log("initJoystick: JoyBody initialized successfully:", JoyBody);
        } catch (e) {
            console.error("initJoystick: Error initializing JoyBody:", e);
        }
    } else {
        // console.log("initJoystick: joyBody canvas element NOT found in DOM. (Expected if not on index.html)"); 
    }
}
