#ifndef HuyangAudio_h
#define HuyangAudio_h

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

class HuyangAudio
{
public:
	HuyangAudio();
	void setup();
	void loop(); // Existing loop for automatic/status handling

	// --- NEW Public Methods for Audio Control ---
	void setVolume(uint8_t volume); // Set volume (0-30)
	void playTrack(uint16_t trackNumber); // Play a specific track by number (1-indexed)
	void pause(); // Pause playback
	void start(); // Resume playback
	void stop();  // Stop playback (reset to beginning of track or silence)
	void nextTrack(); // Play next track
	void previousTrack(); // Play previous track

	// --- NEW Public Getters for Status ---
	uint8_t getVolume(); // Get current volume (0-30)
	uint16_t getCurrentTrack(); // Get current playing track number
	uint16_t getTotalTracks(); // Get total number of tracks found on SD card
	bool isPlaying(); // Check if player is currently playing audio

private:
	unsigned long _currentMillis = 0;
	unsigned long _previousMillis = 0;

	bool _isSerialReady = false;
	bool _isPlayerReady = false;

	DFRobotDFPlayerMini _player;
	SoftwareSerial _audioSerial;

	uint16_t _audioPause = 2000;
	uint16_t _audioItemCount = 0; // Total number of audio files found on SD card
	uint16_t _currentPlayingTrack = 0; // The track number currently playing or last played

	// Flag to indicate if manual control is active (overrides random play)
	bool _manualControlActive = false;
};

#endif
