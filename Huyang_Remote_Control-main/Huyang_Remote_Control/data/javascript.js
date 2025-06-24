// Global variables to hold the current state of the robot's movements and eyes
var automatic = true;

var face_eyes_all = null; // Stores the state for 'all eyes' commands
var face_eyes_left = 3; // Default to blink (HuyangFace::EyeState::Blink)
var face_eyes_right = 3; // Default to blink

var neck_rotate = 0;
var neck_tiltForward = 0;
var neck_tiltSideways = 0;

var body_rotate = 0;
var body_tiltForward = 0;
var body_tiltSideways = 0;

var currentTorsoLightMode = 0; // Default to off

// Calibration values (will be fetched from the server)
var calNeckRotation = 0;
var calNeckTiltForward = 0;
var calNeckTiltSideways = 0;
var calBodyRotation = 0;
var calBodyTiltForward = 0;
var calBodyTiltSideways = 0;
var calMonoclePosition = 0;

// Joystick instances
var JoyNeck;
var JoyNeckX = 0;
var JoyNeckY = 0;

var JoyBody;
var JoyBodyX = 0;
var JoyBodyY = 0;

// --- API Communication Functions ---

/**
 * Sends data to the Arduino WebServer via a POST request.
 * @param {object} data - The JavaScript object to be sent as JSON.
 */
async function sendData(data) {
    try {
        const response = await fetch('/api/post.json', {
            method: 'POST',
            // IMPORTANT: Reverted to 'no-cors' mode as per original working file.
            // This is often required for ESP-based web servers that don't send
            // appropriate CORS headers, preventing the browser from reading the response.
            mode: 'no-cors', 
            cache: 'no-cache',
            headers: {
                'Content-Type': 'application/json'
            },
            referrerPolicy: 'no-referrer',
            body: JSON.stringify(data)
        });

        // In 'no-cors' mode, response.ok is always true and response.json() will fail.
        // We cannot read the response body directly in 'no-cors' mode.
        // The server-side (Arduino) is responsible for updating its state.
        // We'll rely on periodic getServerData calls to update the UI.
        console.log('Data sent. Cannot read response in no-cors mode.');
        // After sending data, immediately try to get the server's updated state
        // to refresh the UI, as the automatic polling might be too slow.
        getServerData(); 
    } catch (error) {
        console.error('Error sending data:', error);
    }
}

/**
 * Fetches the current state from the Arduino WebServer.
 */
async function getServerData() {
    try {
        const response = await fetch('/api/post.json', { // Use the same endpoint for GET to fetch state
            method: 'GET', 
            headers: {
                'Accept': 'application/json' 
            }
        });

        if (!response.ok) {
            // Log full response for debugging non-ok statuses
            const errorText = await response.text();
            console.error(`HTTP error! Status: ${response.status}, Body: ${errorText}`);
            throw new Error(`HTTP error! status: ${response.status}`);
        }

        const result = await response.json();
        console.log('Fetched server data:', result);
        updateUserInterface(result); // Update UI based on fetched state
    } catch (error) {
        console.error('Error fetching data:', error);
    }
}

/**
 * Updates the local UI state based on the data received from the server.
 * This ensures the UI reflects the actual state of the robot.
 * @param {object} data - The JSON object containing the robot's state.
 */
function updateUserInterface(data) {
    // Update automatic toggle button
    automatic = data.automatic;
    const autoButton = document.getElementById('button_automatic');
    if (autoButton) {
        if (automatic) {
            autoButton.innerText = 'Automatic (ON)';
            autoButton.classList.add('selected');
            autoButton.onclick = () => changeAutomatic(false);
        } else {
            autoButton.innerText = 'Manual (OFF)';
            autoButton.classList.remove('selected');
            autoButton.onclick = () => changeAutomatic(true);
        }
    }

    // Update face controls
    if (data.face) {
        // Access directly from data.face, as configured in WebServer.cpp now
        face_eyes_all = data.face.all || 0;
        face_eyes_left = data.face.left || 3; // Default to blink
        face_eyes_right = data.face.right || 3; // Default to blink
        updateFaceButtons();
    }

    // Update neck controls
    if (data.neck) {
        neck_rotate = data.neck.rotate || 0;
        neck_tiltForward = data.neck.tiltForward || 0;
        neck_tiltSideways = data.neck.tiltSideways || 0;
        
        const neckTiltSidewaysSlider = document.getElementById('slider_neckTiltSideways');
        const neckTiltSidewaysValue = document.getElementById('slider_neckTiltSideways_value'); // Get the span
        if (neckTiltSidewaysSlider) {
            neckTiltSidewaysSlider.value = neck_tiltSideways;
            if (neckTiltSidewaysValue) neckTiltSidewaysValue.innerText = neck_tiltSideways; // Update span
        }
        // Joystick positions are updated by the joystick library itself
    }

    // Update body controls
    if (data.body) {
        body_rotate = data.body.rotate || 0;
        body_tiltForward = data.body.tiltForward || 0;
        body_tiltSideways = data.body.tiltSideways || 0;

        const bodyTiltSidewaysSlider = document.getElementById('slider_bodyTiltSideways');
        const bodyTiltSidewaysValue = document.getElementById('slider_bodyTiltSideways_value'); // Get the span
        if (bodyTiltSidewaysSlider) {
            bodyTiltSidewaysSlider.value = body_tiltSideways;
            if (bodyTiltSidewaysValue) bodyTiltSidewaysValue.innerText = body_tiltSideways; // Update span
        }
        // Joystick positions are updated by the joystick library itself
    }

    // Update torso lights
    if (data.torso) {
        currentTorsoLightMode = data.torso.lightMode || 0;
        updateChestLightButtons();
        updateLedVisuals(currentTorsoLightMode);
    }

    // Update calibration sliders (these are now part of the main `data` object, not nested in `neck` or `body`)
    if (data.neck && data.neck.rotation !== undefined) calNeckRotation = data.neck.rotation; // Check specific nested keys if they exist
    if (data.neck && data.neck.tiltForward !== undefined) calNeckTiltForward = data.neck.tiltForward;
    if (data.neck && data.neck.tiltSideways !== undefined) calNeckTiltSideways = data.neck.tiltSideways;
    
    if (data.body && data.body.rotation !== undefined) calBodyRotation = data.body.rotation;
    if (data.body && data.body.tiltForward !== undefined) calBodyTiltForward = data.body.tiltForward;
    if (data.body && data.body.tiltSideways !== undefined) calBodyTiltSideways = data.body.tiltSideways;

    if (data.monocle && data.monocle.position !== undefined) calMonoclePosition = data.monocle.position;

    // Call function to update calibration sliders if on that page
    if (document.getElementById('cal_neck_rotation')) { // Check if calibration page elements exist
        initCalibrationSliders(); // Re-initialize or update sliders with new values
    }
}

// --- Specific Control Functions ---

function sendEyeUpdate(position, action) {
    const data = {
        automatic: false,
        face: {}
    };

    if (position === 'all') {
        data.face.all = action;
    } else {
        data.face.left = face_eyes_left; // Ensure other eye state is carried over
        data.face.right = face_eyes_right; // Ensure other eye state is carried over
        data.face[position] = action;
    }
    sendData(data);
}

function changeAutomatic(newState) {
    const data = {
        automatic: newState
    };
    sendData(data);
}

function sendNeckUpdate() {
    const data = {
        automatic: false,
        neck: {
            rotate: JoyNeck ? JoyNeck.GetX() : neck_rotate, // Use joystick if available, else current value
            tiltForward: JoyNeck ? JoyNeck.GetY() : neck_tiltForward, // Use joystick if available, else current value
            tiltSideways: document.getElementById('slider_neckTiltSideways') ? document.getElementById('slider_neckTiltSideways').value : neck_tiltSideways
        }
    };
    sendData(data);
}

function sendBodyUpdate() {
    const data = {
        automatic: false,
        body: {
            rotate: JoyBody ? JoyBody.GetX() : body_rotate,
            tiltForward: JoyBody ? JoyBody.GetY() : body_tiltForward,
            tiltSideways: document.getElementById('slider_bodyTiltSideways') ? document.getElementById('slider_bodyTiltSideways').value : body_tiltSideways
        }
    };
    sendData(data);
}

function sendChestLightUpdate(mode) {
    const data = {
        automatic: false,
        torso: {
            lightMode: mode
        }
    };
    sendData(data);
}


function sendCalibrationUpdate(part, param, value) {
    const data = {
        automatic: false, // Calibration is always a manual override
        calibration: {
            part: part,
            param: param,
            value: parseInt(value) // Ensure value is an integer
        }
    };
    sendData(data);
}

function sendCalibrationAction(actionType) {
    const data = {
        automatic: false,
        calibration: {
            action: actionType
        }
    };
    sendData(data);
}

function sendCalibrationReset() {
    console.log("Sending calibration reset command.");
    sendCalibrationAction('reset');
}

function sendCalibrationSave() {
    console.log("Sending calibration save command.");
    sendCalibrationAction('save');
}

function sendServosToMiddleAndLock() {
    console.log("Sending command to set servos to middle and lock.");
    sendCalibrationAction('setMiddleAndLock');
}

function sendUnlockServos() {
    console.log("Sending command to unlock servos.");
    sendCalibrationAction('unlockServos');
}


// --- UI Update Helpers ---

function updateFaceButtons() {
    const eyeStates = {
        1: 'open',
        2: 'close',
        3: 'blink',
        4: 'focus',
        5: 'sad',
        6: 'angry'
    };

    // Update 'all eyes' buttons
    Object.values(eyeStates).forEach(stateName => {
        const button = document.getElementById(`eye_all_${stateName}`);
        if (button) {
            if (stateName === eyeStates[face_eyes_all]) {
                button.classList.add('selected');
            } else {
                button.classList.remove('selected');
            }
        }
    });

    // Update left eye buttons
    Object.values(eyeStates).forEach(stateName => {
        const button = document.getElementById(`eye_left_${stateName}`);
        if (button) {
            if (stateName === eyeStates[face_eyes_left]) {
                button.classList.add('selected');
            } else {
                button.classList.remove('selected');
            }
        }
    });

    // Update right eye buttons
    Object.values(eyeStates).forEach(stateName => {
        const button = document.getElementById(`eye_right_${stateName}`);
        if (button) {
            if (stateName === eyeStates[face_eyes_right]) {
                button.classList.add('selected');
            } else {
                button.classList.remove('selected');
            }
        }
    });
}

function updateChestLightButtons() {
    const buttons = document.querySelectorAll('.light-mode-buttons .button');
    buttons.forEach(button => {
        const mode = parseInt(button.onclick.toString().match(/sendChestLightUpdate\((\d+)\)/)[1]);
        if (mode === currentTorsoLightMode) {
            button.classList.add('selected');
        } else {
            button.classList.remove('selected');
        }
    });
}

/**
 * Updates the visual representation of the LEDs on the chest lights page.
 * @param {number} mode - The current light mode (0: Off, 1: On, 2: Warning Blink, etc.).
 */
function updateLedVisuals(mode) {
    const leftLed = document.getElementById('leftLed');
    const rightLed = document.getElementById('rightLed');
    if (!leftLed || !rightLed) return; // Exit if not on the chest lights page

    // Remove all previous color classes
    const colorClasses = ['on-red', 'on-green', 'on-blue', 'on-yellow', 'on-magenta', 'on-cyan', 'on-amber'];
    leftLed.classList.remove(...colorClasses);
    rightLed.classList.remove(...colorClasses);

    // Apply styles based on the mode
    switch (mode) {
        case 0: // Off
            leftLed.style.backgroundColor = '#333';
            rightLed.style.backgroundColor = '#333';
            leftLed.style.boxShadow = 'none';
            rightLed.style.boxShadow = 'none';
            break;
        case 1: // On
            leftLed.classList.add('on-green'); // Example: Solid Green
            rightLed.classList.add('on-green');
            break;
        case 2: // Warning Blink (Red)
            leftLed.classList.add('on-red');
            rightLed.classList.add('on-red');
            // Blinking will be handled by Arduino, but we can set the color here.
            break;
        case 3: // Processing (Blue/Cyan)
            leftLed.classList.add('on-cyan');
            rightLed.classList.add('on-cyan');
            break;
        case 4: // Droid Mode 1 (Amber)
            leftLed.classList.add('on-amber');
            rightLed.classList.add('on-amber');
            break;
        case 5: // Droid Mode 2 (Magenta)
            leftLed.classList.add('on-magenta');
            rightLed.classList.add('on-magenta');
            break;
        default: // Fallback to off
            leftLed.style.backgroundColor = '#333';
            rightLed.style.backgroundColor = '#333';
            leftLed.style.boxShadow = 'none';
            rightLed.style.boxShadow = 'none';
            break;
    }
}


// --- Initialization Functions ---

/**
 * Initializes the main system. Fetches server data and loads dynamic content.
 */
function systemInit() {
    console.log('systemInit started');

    // IMPORTANT: Re-introduce continuous polling for updates
    // This fetches the current state from the server every 2 seconds,
    // which is crucial for automatic mode display and other state synchronization.
    setInterval(getServerData, 2000); 

    let timeToWait = 0;
    const timeToPause = 200; // Small delay between loading sections

    if (document.getElementById('container_face') != null) {
        setTimeout(function() {
            loadContainer('index.face.html', 'container_face');
        }, timeToWait);
        timeToWait += timeToPause;
    }

    if (document.getElementById('container_neck') != null) {
        setTimeout(function() {
            loadContainer('index.neck.html', 'container_neck');
        }, timeToWait);
        timeToWait += timeToPause;
    }

    if (document.getElementById('container_body') != null) {
        setTimeout(function() {
            loadContainer('index.body.html', 'container_body');
        }, timeToWait);
        timeToWait += timeToPause;
    }
}

/**
 * Loads HTML content into a specified container element.
 * @param {string} url - The URL of the HTML fragment to load.
 * @param {string} elementId - The ID of the HTML element to load content into.
 */
async function loadContainer(url, elementId) {
    try {
        const response = await fetch(url);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const html = await response.text();
        const container = document.getElementById(elementId);
        if (container) {
            container.innerHTML = html;
            container.removeAttribute('onclick'); // Remove the onclick handler after loading content
            initJoystick(); // Initialize joysticks after content is loaded
            
            // Re-fetch server data to ensure newly loaded UI elements are correctly updated
            // This is especially important for buttons, sliders, etc.
            // This call should happen immediately after container content is loaded,
            // the setInterval will handle continuous updates.
            getServerData(); 
        }
    } catch (error) {
        console.error(`Error loading container ${elementId} from ${url}:`, error);
    }
}

/**
 * Initializes the joystick instances if their canvas elements are present.
 */
function initJoystick() {
    const fillColor = '#fdd835'; // Amber color for joystick
    const strokeColor = '#b90'; // Darker amber/orange

    const joystickSettings = {
        autoReturnToCenter: true,
        internalFillColor: fillColor,
        internalLineWidth: 2,
        internalStrokeColor: strokeColor,
        externalLineWidth: 2,
        externalStrokeColor: strokeColor
    };

    // Initialize Neck Joystick
    if (JoyNeck == null && document.getElementById('joyNeck') != null) {
        JoyNeck = new JoyStick('joyNeck', joystickSettings, function (stickData) {
            // Only send update if values have changed significantly to reduce traffic
            if (Math.abs(JoyNeckX - stickData.x) > 1 || Math.abs(JoyNeckY - stickData.y) > 1) {
                JoyNeckX = stickData.x;
                JoyNeckY = stickData.y;
                sendNeckUpdate(); // Corrected: Neck joystick sends neck update
            }
        });
    }

    // Initialize Body Joystick
    if (JoyBody == null && document.getElementById('joyBody') != null) {
        JoyBody = new JoyStick('joyBody', joystickSettings, function (stickData) {
            if (Math.abs(JoyBodyX - stickData.x) > 1 || Math.abs(JoyBodyY - stickData.y) > 1) {
                JoyBodyX = stickData.x;
                JoyBodyY = stickData.y;
                sendBodyUpdate();
            }
        });
    }
}

/**
 * Initializes and updates the calibration sliders and their displayed values.
 * This function should be called when the calibration page loads or after receiving
 * updated calibration data from the server.
 */
function initCalibrationSliders() {
    // Neck Rotation
    const calNeckRotSlider = document.getElementById('cal_neck_rotation');
    const calNeckRotValue = document.getElementById('cal_neck_rotation_value');
    if (calNeckRotSlider && calNeckRotValue) {
        calNeckRotSlider.value = calNeckRotation;
        calNeckRotValue.innerText = calNeckRotation;
    }

    // Neck Tilt Forward
    const calNeckTFSelSlider = document.getElementById('cal_neck_tilt_forward');
    const calNeckTFSelValue = document.getElementById('cal_neck_tilt_forward_value');
    if (calNeckTFSelSlider && calNeckTFSelValue) {
        calNeckTFSelSlider.value = calNeckTiltForward;
        calNeckTFSelValue.innerText = calNeckTiltForward;
    }

    // Neck Tilt Sideways
    const calNeckTSSelSlider = document.getElementById('cal_neck_tilt_sideways');
    const calNeckTSSelValue = document.getElementById('cal_neck_tilt_sideways_value');
    if (calNeckTSSelSlider && calNeckTSSelValue) {
        calNeckTSSelSlider.value = calNeckTiltSideways;
        calNeckTSSelValue.innerText = calNeckTiltSideways;
    }

    // Body Rotation
    const calBodyRotSlider = document.getElementById('cal_body_rotation');
    const calBodyRotValue = document.getElementById('cal_body_rotation_value');
    if (calBodyRotSlider && calBodyRotValue) {
        calBodyRotSlider.value = calBodyRotation;
        calBodyRotValue.innerText = calBodyRotation;
    }

    // Body Tilt Forward
    const calBodyTFSelSlider = document.getElementById('cal_body_tilt_forward');
    const calBodyTFSelValue = document.getElementById('cal_body_tilt_forward_value');
    if (calBodyTFSelSlider && calBodyTFSelValue) {
        calBodyTFSelSlider.value = calBodyTiltForward;
        calBodyTFSelValue.innerText = calBodyTiltForward;
    }

    // Body Tilt Sideways
    const calBodyTSSelSlider = document.getElementById('cal_body_tilt_sideways');
    const calBodyTSSelValue = document.getElementById('cal_body_tilt_sideways_value');
    if (calBodyTSSelSlider && calBodyTSSelValue) {
        calBodyTSSelSlider.value = calBodyTiltSideways;
        calBodyTSSelValue.innerText = calBodyTiltSideways;
    }

    // Monocle Position
    const calMonoclePosSlider = document.getElementById('cal_monocle_position');
    const calMonoclePosValue = document.getElementById('cal_monocle_position_value');
    if (calMonoclePosSlider && calMonoclePosValue) {
        calMonoclePosSlider.value = calMonoclePosition;
        calMonoclePosValue.innerText = calMonoclePosition;
    }
}


// Event listener for when the entire page content is loaded
document.addEventListener('DOMContentLoaded', function() {
    // systemInit() is called from window.onload in index.html, so it handles the main sections.
    // For specific pages, we can call their init functions directly if they are the main entry.
    // Example: if a user directly navigates to calibration.html
    if (document.body.id === 'calibration-page') { // Add an ID to your body tag in calibration.html
        getServerData(); // Fetch data and then initCalibrationSliders will be called by updateUserInterface
    } else if (document.body.id === 'chestlights-page') { // Add an ID to your body tag in chestlights.html
        getServerData(); // Fetch data and then updateLedVisuals will be called by updateUserInterface
    }
});
