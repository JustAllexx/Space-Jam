#include <AL/alc.h>
#include <AL/al.h>
#include <sndfile.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>

#pragma once
class AudioManager
{
private:
	ALCdevice* device;
	ALCdevice* captureDev;
	ALCcontext* context;
	//TODO Add multiple audio sources, probably for GUI stuff
	ALuint source;
	std::unordered_map<std::string, ALuint> audioBuffers;
	std::vector<int16_t> captureBuffer;
	bool startedPlaying = false;

	void setupDevice();
	void setupSource();
public:
	AudioManager();
	~AudioManager();
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = default;

	ALuint addAudioBuffer(std::string_view, std::string_view audioIdentifier);
	void playAudioBuffer(std::string_view audioIdentifier);
	void StartCapture();
	void updateFrequency(double &note, double &volume);
	float getPlayPos();
	bool isPlaying() const noexcept {return startedPlaying;}


	static float getHeightOfNote(int ind, float fovy, float dist);
};

// RAII Class for loading audio using the libsound c library
class SoundFile {
private:
	SNDFILE* file{};
	SF_INFO info{};

public:
	int getChannels() const noexcept {return info.channels;}
	int getSampleRate() const noexcept {return info.samplerate;}
	sf_count_t getFrameCount() const noexcept {return info.frames;}
	int getFormat() const noexcept {return info.format;}

	void readSamples(std::vector<short>& sampleBuffer) {
		sf_readf_short(file, sampleBuffer.data(), info.frames);
	}

	SoundFile(const char* filepath) {
		file = sf_open(filepath, SFM_READ, &info);
		if (!file) {throw std::runtime_error(sf_strerror(nullptr));}
	}
	~SoundFile() {
		if (file) {sf_close(file);}
	}
	SoundFile (const SoundFile&) = delete;
	SoundFile& operator=(const SoundFile&) = delete;
};

