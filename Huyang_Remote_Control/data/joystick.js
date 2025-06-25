    /*
     * Name          : joy.js
     * @author       : Roberto D'Amico (Bobboteck)
     * Last modified : 09.06.2020
     * Revision      : 1.1.6
     *
     * Modification History:
     * Date         Version     Modified By     Description
     * 2021-12-21   2.0.0       Roberto D'Amico New version of the project that integrates the callback functions, while 
     * maintaining compatibility with previous versions. Fixed Issue #27 too, 
     * thanks to @artisticfox8 for the suggestion.
     * 2020-06-09   1.1.6       Roberto D'Amico Fixed Issue #10 and #11
     * 2020-04-20   1.1.5       Roberto D'Amico Correct: Two sticks in a row, thanks to @liamw9534 for the suggestion
     * 2020-04-03               Roberto D'Amico Correct: InternalRadius when change the size of canvas, thanks to 
     * @vanslipon for the suggestion
     * 2020-01-07   1.1.4       Roberto D'Amico Close #6 by implementing a new parameter to set the functionality of 
     * auto-return to 0 position
     * 2019-11-18   1.1.3       Roberto D'Amico Close #5 correct indication of East direction
     * 2019-11-12   1.1.2       Roberto D'Amico Removed Fix #4 incorrectly introduced and restored operation with touch 
     * devices
     * 2019-11-12   1.1.1       Roberto D'Amico Fixed Issue #4 - Now JoyStick work in any position in the page, not only 
     * at 0,0
     * * The MIT License (MIT)
     *
     * This file is part of the JoyStick Project (https://github.com/bobboteck/JoyStick).
     *	Copyright (c) 2015 Roberto D'Amico (Bobboteck).
     *
     * Permission is hereby granted, free of charge, to any person obtaining a copy
     * of this software and associated documentation files (the "Software"), to deal
     * in the Software without restriction, including without limitation the rights
     * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
     * copies of the Software, and to permit persons to whom the Software is
     * furnished to do so, subject to the following conditions:
     * * The above copyright notice and this permission notice shall be included in all
     * copies or substantial portions of the Software.
     *
     * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
     * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
     * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
     * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
     * LIABILITY, WHETHER IN AN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
     * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
     * SOFTWARE.
     */

    // This checks if JoyStick is already defined before attempting to declare it.
    if (typeof JoyStick === 'undefined') {
        var JoyStick = (function(containerid, parameters, callback) { 
            "use strict";
            console.log(`JoyStick: Constructor called for container ID: ${containerid}`);

            let objContainer = document.getElementById(containerid);
            if (!objContainer) {
                console.error("JoyStick: Container element with ID '" + containerid + "' not found. Cannot initialize joystick.");
                return null;
            }
            console.log("JoyStick: Container element found:", objContainer);

            let canvas = document.createElement("canvas");
            let context = canvas.getContext("2d");
            let pressed = 0;
            let circumference = 2 * Math.PI;

            // Options with defaults
            let title = (typeof parameters.title === "undefined" ? "joystick" : parameters.title),
                width = (typeof parameters.width === "undefined" ? 0 : parameters.width),
                height = (typeof parameters.height === "undefined" ? 0 : parameters.height),
                internalFillColor = (typeof parameters.internalFillColor === "undefined" ? "#00AA00" : parameters.internalFillColor),
                internalLineWidth = (typeof parameters.internalLineWidth === "undefined" ? 2 : parameters.internalLineWidth),
                internalStrokeColor = (typeof parameters.internalStrokeColor === "undefined" ? "#003300" : parameters.internalStrokeColor),
                externalLineWidth = (typeof parameters.externalLineWidth === "undefined" ? 2 : parameters.externalLineWidth),
                externalStrokeColor = (typeof parameters.externalStrokeColor === "undefined" ? "#008000" : parameters.externalStrokeColor),
                autoReturnToCenter = (typeof parameters.autoReturnToCenter === "undefined" ? true : parameters.autoReturnToCenter);

            callback = callback || function(StickStatus) {};

            objContainer.style.touchAction = "none"; // Fixing Unable to preventDefault inside passive event listener

            canvas.id = title;
            // Set canvas dimensions based on container or parameters
            if(width === 0) { canvas.width = objContainer.clientWidth; } else { canvas.width = width; }
            if(height === 0) { canvas.height = objContainer.clientHeight; } else { canvas.height = height; }
            
            objContainer.appendChild(canvas);
            console.log(`JoyStick: Canvas created and appended. Dimensions: ${canvas.width}x${canvas.height}`);

            // Recalculate radii based on actual canvas size
            let internalRadius = (canvas.width - ((canvas.width/2)+10))/2; 
            let maxMoveStick = internalRadius + 5;
            let externalRadius = internalRadius + 30;
            let centerX = canvas.width / 2;
            let centerY = canvas.height / 2;
            let directionHorizontalLimitPos = canvas.width / 10;
            let directionHorizontalLimitNeg = directionHorizontalLimitPos * -1;
            let directionVerticalLimitPos = canvas.height / 10;
            let directionVerticalLimitNeg = directionVerticalLimitPos * -1;

            console.log(`JoyStick: Calculated radii - internal: ${internalRadius}, external: ${externalRadius}, maxMove: ${maxMoveStick}`);
            console.log(`JoyStick: Center X: ${centerX}, Center Y: ${centerY}`);


            // Used to save current position of stick
            var movedX = centerX;
            var movedY = centerY;

            if("ontouchstart" in document.documentElement) {
                canvas.addEventListener("touchstart", onTouchStart, false);
                document.addEventListener("touchmove", onTouchMove, false);
                document.addEventListener("touchend", onTouchEnd, false);
            } else {
                canvas.addEventListener("mousedown", onMouseDown, false);
                document.addEventListener("mousemove", onMouseMove, false);
                document.addEventListener("mouseup", onMouseUp, false);
            }

            drawExternal();
            drawInternal();

            function drawExternal() {
                // console.log("JoyStick: Drawing external circle.");
                context.beginPath();
                context.arc(centerX, centerY, externalRadius, 0, circumference, false);
                context.lineWidth = externalLineWidth;
                context.strokeStyle = externalStrokeColor;
                context.stroke();
            }

            function drawInternal() {
                // console.log(`JoyStick: Drawing internal stick at (${movedX}, ${movedY}).`);
                context.beginPath();
                
                let drawX = movedX;
                let drawY = movedY;

                // Clamp to prevent drawing outside the external circle (using maxMoveStick as boundary)
                let dx = drawX - centerX;
                let dy = drawY - centerY;
                let distance = Math.sqrt(dx * dx + dy * dy);

                if (distance > maxMoveStick) {
                    let angle = Math.atan2(dy, dx);
                    drawX = centerX + maxMoveStick * Math.cos(angle);
                    drawY = centerY + maxMoveStick * Math.sin(angle);
                }

                context.arc(drawX, drawY, internalRadius, 0, circumference, false);
                var grd = context.createRadialGradient(centerX, centerY, 5, centerX, centerY, 200);
                grd.addColorStop(0, internalFillColor);
                grd.addColorStop(1, internalStrokeColor);
                context.fillStyle = grd;
                context.fill();
                context.lineWidth = internalLineWidth;
                context.strokeStyle = internalStrokeColor;
                context.stroke();
            }

            let touchId = null;
            function onTouchStart(event) {
                pressed = 1;
                touchId = event.targetTouches[0].identifier;
                let rect = canvas.getBoundingClientRect();
                movedX = event.targetTouches[0].clientX - rect.left;
                movedY = event.targetTouches[0].clientY - rect.top;
                context.clearRect(0, 0, canvas.width, canvas.height); 
                drawExternal();
                drawInternal();
                callback(StickStatus(movedX, movedY, centerX, centerY, maxMoveStick, getCardinalDirection()));
                event.preventDefault();
            }

            function onTouchMove(event) {
                if(pressed === 1 && event.targetTouches[0].target === canvas) {
                    let rect = canvas.getBoundingClientRect();
                    movedX = event.targetTouches[0].clientX - rect.left;
                    movedY = event.targetTouches[0].clientY - rect.top;
                    context.clearRect(0, 0, canvas.width, canvas.height);
                    drawExternal();
                    drawInternal();
                    callback(StickStatus(movedX, movedY, centerX, centerY, maxMoveStick, getCardinalDirection()));
                    event.preventDefault();
                }
            }

            function onTouchEnd(event) {
                if (event.changedTouches[0].identifier !== touchId) return;

                pressed = 0;
                if(autoReturnToCenter) {
                    movedX = centerX;
                    movedY = centerY;
                }
                context.clearRect(0, 0, canvas.width, canvas.height);
                drawExternal();
                drawInternal();
                callback(StickStatus(movedX, movedY, centerX, centerY, maxMoveStick, getCardinalDirection()));
            }

            function onMouseDown(event) {
                pressed = 1;
                let rect = canvas.getBoundingClientRect();
                movedX = event.clientX - rect.left;
                movedY = event.clientY - rect.top;
                context.clearRect(0, 0, canvas.width, canvas.height);
                drawExternal();
                drawInternal();
                callback(StickStatus(movedX, movedY, centerX, centerY, maxMoveStick, getCardinalDirection()));
            }

            function onMouseMove(event) {
                if(pressed === 1) {
                    let rect = canvas.getBoundingClientRect();
                    movedX = event.clientX - rect.left;
                    movedY = event.clientY - rect.top;
                    context.clearRect(0, 0, canvas.width, canvas.height);
                    drawExternal();
                    drawInternal();
                    callback(StickStatus(movedX, movedY, centerX, centerY, maxMoveStick, getCardinalDirection()));
                }
            }

            function onMouseUp(event) {
                pressed = 0;
                if(autoReturnToCenter) {
                    movedX = centerX;
                    movedY = centerY;
                }
                context.clearRect(0, 0, canvas.width, canvas.height);
                drawExternal();
                drawInternal();
                callback(StickStatus(movedX, movedY, centerX, centerY, maxMoveStick, getCardinalDirection()));
            }

            function getCardinalDirection() {
                let result = "";
                let orizontal = movedX - centerX;
                let vertical = movedY - centerY;
                
                if(vertical >= directionVerticalLimitNeg && vertical <= directionVerticalLimitPos) {
                    result = "C";
                }
                if(vertical < directionVerticalLimitNeg) {
                    result = "N";
                }
                if(vertical > directionVerticalLimitPos) {
                    result = "S";
                }
                
                if(orizontal < directionHorizontalLimitNeg) {
                    if(result === "C") { 
                        result = "W";
                    } else {
                        result += "W";
                    }
                }
                if(orizontal > directionHorizontalLimitPos) {
                    if(result === "C") { 
                        result = "E";
                    } else {
                        result += "E";
                    }
                }
                return result;
            }

            this.GetWidth = function () { return canvas.width; };
            this.GetHeight = function () { return canvas.height; };
            this.GetPosX = function () { return movedX; };
            this.GetPosY = function () { return movedY; };
            
            this.GetX = function () {
                let currentX = movedX - centerX;
                return (100 * (currentX / maxMoveStick)).toFixed(0);
            };
            this.GetY = function () {
                let currentY = movedY - centerY;
                return ((100 * (currentY / maxMoveStick)) * -1).toFixed(0);
            };
            this.GetValues = function () {
                return { x: parseInt(this.GetX()), y: parseInt(this.GetY()) };
            };
            this.GetDir = function() { return getCardinalDirection(); };

            function StickStatus(xPosition, yPosition, centerX, centerY, maxMoveStick, cardinalDirection) {
                return {
                    xPosition: xPosition,
                    yPosition: yPosition,
                    x: (100 * ((xPosition - centerX) / maxMoveStick)).toFixed(0),
                    y: ((100 * ((yPosition - centerY) / maxMoveStick)) * -1).toFixed(0),
                    cardinalDirection: cardinalDirection
                };
            }
        });
    }

    if (!Number.prototype.toFixed) {
        Number.prototype.toFixed = function(precision) {
            var power = Math.pow(10, precision || 0);
            return (Math.round(this * power) / power).toString();
        };
    }
    