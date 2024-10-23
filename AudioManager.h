#include <AL/alc.h>
#include <AL/al.h>
#include <sndfile.h>
#include <Windows.h>
#include <valarray>
#include <complex>
#include <chrono>
#include <vector>
#include <map>
#include <string>
#include "YIN.h"

#pragma once
class AudioManager
{
private:
	ALCdevice* device;
	ALCdevice* captureDev;
	ALCcontext* context;
	std::vector<ALuint> audioBuffers;

	ALuint playingBuffer = 0;
	//The main code checks if a buffer is finished playing by getting the second offset
	//The second offset is 0 when the song ends
	//But it is also 0 when the song starts, so create a boolean that keeps track of if the play command has just been sent
	//That then becomes false as soon as the play position increments past 0
public:

	ALuint source;
	bool startedPlaying = false;
	AudioManager();
	void setupDevice();
	void setupSource();
	ALuint addAudioBuffer(const char* path);
	void playAudioBuffer(ALuint buffer);
	void StartCapture();
	void updateFrequency(float dt, double &note, double &volume);
	float getPlayPos();

	static float getHeightOfNote(int ind, float fovy, float dist);
};

