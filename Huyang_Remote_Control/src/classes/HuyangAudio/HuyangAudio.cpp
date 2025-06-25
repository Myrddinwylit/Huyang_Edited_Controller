    #include "HuyangAudio.h"

    // STRICTLY RETAINING ORIGINAL PIN ASSIGNMENTS as requested by user
    #define AudioSerialPort_TX -1 
    #define AudioSerialPort_RX 12

    // Forward declaration of printDetail function (already exists)
    void printDetail(uint8_t type, int value);

    // Constructor: Initializes the SoftwareSerial as per original file.
    HuyangAudio::HuyangAudio() : _audioSerial(AudioSerialPort_RX, AudioSerialPort_TX)
    {
        _audioSerial.begin(9600, SWSERIAL_8N1, AudioSerialPort_RX, AudioSerialPort_TX);
    }

    // Setup method: Initializes Serial, SoftwareSerial, and DFPlayer Mini.
    void HuyangAudio::setup()
    {
        Serial.println("HuyangAudio setup");

        if (!_audioSerial)
        { 
            Serial.println("SoftwareSerial failed to initialize for DFPlayer. Check pin configuration!");
            while (1) { delay(1000); } 
        } else {
            _isSerialReady = true;
            Serial.println("SoftwareSerial for DFPlayer is ready.");
        }

        if (_isSerialReady && !_player.begin(_audioSerial, /*isACK = */ false, /*doReset = */ false))
        {
            Serial.println("Connecting to DFPlayer Mini failed! Check wiring and SD card.");
        }
        else if (_isSerialReady)
        {
            _isPlayerReady = true;
            Serial.println("DFPlayer Mini IS READY.");

            _player.volume(25); 
            Serial.printf("DFPlayer initial volume set to %d.\n", _player.readVolume());

            _audioItemCount = _player.readFileCounts();
            if (_audioItemCount > 0) {
                Serial.printf("Found %d audio files on SD card.\n", _audioItemCount);
            } else {
                Serial.println("No audio files found on SD card or failed to read count.");
                _audioItemCount = 0; 
            }

            // Get current track number from player (if any)
            _currentPlayingTrack = _player.readCurrentFileNumber(); // FIX: Changed to readCurrentFileNumber()
            _player.stop(); 
        }
    }

    // Main loop function: Handles automatic playback if manual control is not active.
    void HuyangAudio::loop()
    {
        if (!_isSerialReady || !_isPlayerReady) {
            return; 
        }

        // Always process DFPlayer feedback if available
        if (_player.available()) {
            printDetail(_player.readType(), _player.read());
            // If DFPlayer finishes playing a track, its state will reflect it.
            // We can also update our internal _currentPlayingTrack if play finished message received.
            if (_player.readType() == DFPlayerPlayFinished) {
                _currentPlayingTrack = 0; // Or increment if auto-play next is desired
                Serial.println("Track finished playing.");
            }
        }

        // Check player state to determine if it's actually playing.
        // DFRobotDFPlayerMini's readState() often returns 0x41 when playing,
        // and 0x40 when paused, 0x42 when stopped.
        // The library itself doesn't always provide a simple isPlaying() boolean.
        bool currentlyPlaying = (_player.readState() == 0x41); // 0x41 typically means playing

        // Only engage in random playback if manual control is NOT active AND player is not currently playing
        if (!_manualControlActive)
        {
            _currentMillis = millis();

            if (_currentMillis < _previousMillis) {
                _previousMillis = 0;
            }

            // Only play if enough time has passed AND player is not playing AND we have tracks
            if (_currentMillis - _previousMillis >= _audioPause && !currentlyPlaying)
            {
                Serial.println("DFPlayer is NOT playing (in automatic mode). Initiating random play...");
                _previousMillis = _currentMillis; 

                if (_audioItemCount == 0)
                {
                    int count = _player.readFileCounts();
                    if (count > 0) {
                        _audioItemCount = count;
                        Serial.printf("Updated audioItemCount: %d\n", count);
                    } else {
                        Serial.println("No audio files found for random play. Skipping.");
                        return; 
                    }
                }

                if (_audioItemCount > 0)
                {
                    uint16_t randomItemNumber = random(1, _audioItemCount + 1); 

                    if (randomItemNumber == 8) { randomItemNumber = randomItemNumber + 1; }

                    Serial.printf("Playing random item Number %d of %d items.\n", randomItemNumber, _audioItemCount);
                    _player.play(randomItemNumber);
                    _currentPlayingTrack = randomItemNumber; 

                    _audioPause = 2000 + (random(10, 50) * 100);
                }
            }
            // If player *is* playing (e.g., just started automatically) and _currentPlayingTrack is unknown,
            // try to update it.
            else if (currentlyPlaying && _currentPlayingTrack == 0) {
                _currentPlayingTrack = _player.readCurrentFileNumber(); // FIX: Changed to readCurrentFileNumber()
            }
        } else {
            // Manual control is active, if player starts playing manually and _currentPlayingTrack is unknown, update it.
            if (currentlyPlaying && _currentPlayingTrack == 0) {
                _currentPlayingTrack = _player.readCurrentFileNumber(); // FIX: Changed to readCurrentFileNumber()
            }
        }
    }

    // --- Implementation of NEW Public Methods for Audio Control ---

    void HuyangAudio::setVolume(uint8_t volume) {
        if (_isPlayerReady) {
            if (volume > 30) volume = 30;
            _player.volume(volume);
            Serial.printf("DFPlayer volume set to: %d\n", volume);
        } else {
            Serial.println("DFPlayer not ready to set volume.");
        }
    }

    void HuyangAudio::playTrack(uint16_t trackNumber) {
        if (_isPlayerReady) {
            if (trackNumber > 0 && trackNumber <= _audioItemCount) {
                _manualControlActive = true; 
                _player.play(trackNumber);
                _currentPlayingTrack = trackNumber; 
                Serial.printf("DFPlayer playing track: %d\n", trackNumber);
            } else {
                Serial.printf("Invalid track number %d. Total tracks: %d.\n", trackNumber, _audioItemCount);
            }
        } else {
            Serial.println("DFPlayer not ready to play track.");
        }
    }

    void HuyangAudio::pause() {
        if (_isPlayerReady) {
            _player.pause();
            _manualControlActive = true; 
            Serial.println("DFPlayer paused.");
        } else {
            Serial.println("DFPlayer not ready to pause.");
        }
    }

    void HuyangAudio::start() {
        if (_isPlayerReady) {
            _player.start(); 
            _manualControlActive = true; 
            Serial.println("DFPlayer resumed.");
        } else {
            Serial.println("DFPlayer not ready to start/resume.");
        }
    }

    void HuyangAudio::stop() {
        if (_isPlayerReady) {
            _player.stop();
            _manualControlActive = true; 
            _currentPlayingTrack = 0; 
            Serial.println("DFPlayer stopped.");
        } else {
            Serial.println("DFPlayer not ready to stop.");
        }
    }

    void HuyangAudio::nextTrack() {
        if (_isPlayerReady) {
            _manualControlActive = true;
            _player.next();
            _currentPlayingTrack++;
            if (_currentPlayingTrack > _audioItemCount) { 
                _currentPlayingTrack = 1;
            }
            Serial.printf("DFPlayer playing next track. Assumed track: %d\n", _currentPlayingTrack);
        } else {
            Serial.println("DFPlayer not ready for next track.");
        }
    }

    void HuyangAudio::previousTrack() {
        if (_isPlayerReady) {
            _manualControlActive = true;
            _player.previous();
            _currentPlayingTrack--;
            if (_currentPlayingTrack < 1) { 
                _currentPlayingTrack = _audioItemCount;
            }
            Serial.printf("DFPlayer playing previous track. Assumed track: %d\n", _currentPlayingTrack);
        } else {
            Serial.println("DFPlayer not ready for previous track.");
        }
    }

    // --- Implementation of NEW Public Getters for Status ---

    uint8_t HuyangAudio::getVolume() {
        if (_isPlayerReady) {
            return _player.readVolume();
        }
        return 0; 
    }

    uint16_t HuyangAudio::getCurrentTrack() {
        if (_isPlayerReady) {
            // Prioritize our stored track number if manual control is active or we have a valid stored track.
            // Otherwise, try to get the actual playing track from the player.
            if (_manualControlActive && _currentPlayingTrack > 0) {
                return _currentPlayingTrack;
            }
            uint16_t actualTrack = _player.readCurrentFileNumber(); // FIX: Changed to readCurrentFileNumber()
            if (actualTrack > 0) {
                _currentPlayingTrack = actualTrack; 
            }
            return _currentPlayingTrack; 
        }
        return 0; 
    }

    uint16_t HuyangAudio::getTotalTracks() {
        if (_isPlayerReady) {
            return _audioItemCount; 
        }
        return 0; 
    }

    bool HuyangAudio::isPlaying() {
        if (_isPlayerReady) {
            return (_player.readState() == 0x41); // FIX: Check state for playing (0x41)
        }
        return false; 
    }


    // The existing printDetail function (unchanged)
    void printDetail(uint8_t type, int value)
    {
        switch (type)
        {
        case TimeOut:
            Serial.println(F("Time Out!"));
            break;
        case WrongStack:
            Serial.println(F("Stack Wrong!"));
            break;
        case DFPlayerCardInserted:
            Serial.println(F("Card Inserted!"));
            break;
        case DFPlayerCardRemoved:
            Serial.println(F("Card Removed!"));
            break;
        case DFPlayerCardOnline:
            Serial.println(F("Card Online!"));
            break;
        case DFPlayerUSBInserted:
            Serial.println("USB Inserted!");
            break;
        case DFPlayerUSBRemoved:
            Serial.println("USB Removed!");
            break;
        case DFPlayerPlayFinished:
            Serial.print(F("Number:"));
            Serial.print(value);
            Serial.println(F(" Play Finished!"));
            break;
        case DFPlayerError:
            Serial.print(F("DFPlayerError:"));
            switch (value)
            {
            case Busy:
                Serial.println(F("Card not found"));
                break;
            case Sleeping:
                Serial.println(F("Sleeping"));
                break;
            case SerialWrongStack:
                Serial.println(F("Get Wrong Stack"));
                break;
            case CheckSumNotMatch:
                Serial.println(F("Check Sum Not Match"));
                break;
            case FileIndexOut:
                Serial.println(F("File Index Out of Bound"));
                break;
            case FileMismatch:
                Serial.println(F("Cannot Find File"));
                break;
            case Advertise:
                Serial.println(F("In Advertise"));
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    