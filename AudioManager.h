#include <AL/alc.h>
#include <AL/al.h>
#include <sndfile.h>
#include <string>
#include <string_view>
#include <unordered_map>
//#include <string>

#pragma once
class AudioManager
{
private:
	ALCdevice* device;
	ALCdevice* captureDev;
	ALCcontext* context;
	std::unordered_map<std::string, ALuint> audioBuffers;

	//The main code checks if a buffer is finished playing by getting the second offset
	//The second offset is 0 when the song ends
	//But it is also 0 when the song starts, so create a boolean that keeps track of if the play command has just been sent
	//That then becomes false as soon as the play position increments past 0

	void setupDevice();
	void setupSource();
public:

	ALuint source;
	bool startedPlaying = false;
	AudioManager();
	ALuint addAudioBuffer(std::string_view, std::string_view audioIdentifier);
	void playAudioBuffer(std::string_view audioIdentifier);
	void StartCapture();
	void updateFrequency(double &note, double &volume);
	float getPlayPos();

	static float getHeightOfNote(int ind, float fovy, float dist);
};

