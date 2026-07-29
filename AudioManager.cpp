#include "AudioManager.h"
#include "YIN.h"

#include <al.h>
#include <alc.h>
#include <cstddef>
#include <iostream>
#include <limits>
#include <sndfile.h>
#include <stdexcept>
#include <string>
#include <valarray>
#include <complex>

const ALCuint rate = 44100;
const ALCuint size = 1024;
const size_t captureBufferSize = 22050;
const std::vector<std::string> notes = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };

AudioManager::AudioManager() {
	captureBuffer.resize(captureBufferSize);
	//Connect to the two audio devices (connects to the microphone and connects to the speakers)
	setupDevice();
	//Create an OpenAL source that can start playing audio
	setupSource();
}

AudioManager::~AudioManager() {
	alDeleteSources(1, &source);
	for (const auto& [_, bufferID] : audioBuffers) {
		alDeleteBuffers(1, &bufferID);
	}

	alcDestroyContext(context);
	alcCloseDevice(device);
	alcCloseDevice(captureDev);
}

//Creates an OpenAl source with specific properties
void AudioManager::setupSource() {
	alGenSources(1, &source);
	alSourcef(source, AL_PITCH, 1.f);
	alSourcef(source, AL_GAIN, 1.f);
	alSource3f(source, AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(source, AL_VELOCITY, 0.f, 0.f, 0.f);
	alSourcei(source, AL_LOOPING, false);
}

void AudioManager::setupDevice() {
	const ALCchar* devices;
	
	device = alcOpenDevice(nullptr);
	if (!device) { throw std::runtime_error("Couldn't open sound device");}

	context = alcCreateContext(device, nullptr);
	if (!context) { throw std::runtime_error("Failed to create context"); }

	alcMakeContextCurrent(context);
	devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	//checks that a device was found
	if (devices == NULL) {
		throw std::runtime_error("Input device not found");
	}
	//Opens the capture device in "Mono" format, this means that only one integer will be recorder per sample (and it will be of type integer 16 or 16 bits).
	captureDev = alcCaptureOpenDevice(NULL, rate, AL_FORMAT_MONO16, size);

	if (captureDev == NULL) {
		throw std::runtime_error("Failed to open capture device");
	}
}

//Play audio
void AudioManager::playAudioBuffer(std::string_view audioIdentifier) {
	//Buffers the audio content into the source
	std::string audioKey(audioIdentifier);
	ALint buffer = static_cast<ALint>(audioBuffers[audioKey]);
	alSourcei(source, AL_BUFFER, buffer);
	//Starts playing
	alSourcePlay(source);
	startedPlaying = true;
}

ALuint AudioManager::addAudioBuffer(std::string_view path, std::string_view audioIdentifier) {
	std::string path_str(path);
	SoundFile soundFile = SoundFile(path_str.c_str());
	int channels = soundFile.getChannels();
	sf_count_t frames = soundFile.getFrameCount();
	int samplerate = soundFile.getSampleRate();

	//Find out what format the audio is in
	ALenum format = AL_NONE;
	if (channels == 1) { format = AL_FORMAT_MONO16; }
	else if (channels == 2) { format = AL_FORMAT_STEREO16; }
	else { 
		std::cout << "Incorrect Format: more than 2 channels" << std::endl; 
		return 0; 
	}

	//How big the filesize is going to be in bytes
	size_t fileSize = static_cast<size_t>(frames) * static_cast<size_t>(channels) * sizeof(short);
	std::vector<short> memory(fileSize);

	//Buffer in the soundfile to that memory pointer
	soundFile.readSamples(memory);
	if (fileSize >= std::numeric_limits<int>::max()) {
		throw std::runtime_error("File size is too big to allocate OpenAL buffer");
	}
	int bufferSize = static_cast<int>(fileSize);

	//Buffer it into an openAl buffer which can then be played by a source
	ALuint buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, memory.data(), bufferSize, samplerate);

	std::string audioKey(audioIdentifier);
	audioBuffers.insert({audioKey, buffer});
	return buffer;
}

//Starts capturing audio using the capture device
void AudioManager::StartCapture() {
	alcCaptureStart(captureDev);
}

//Get the ammount of seconds through an audio the source is
float AudioManager::getPlayPos() {
	float pos = 0;
	alGetSourcef(source, AL_SEC_OFFSET, &pos);
	//Other functions take pos = 0.f to mean the audio has ended, however pos also equals 0, on start
	//This makes sure that behaviour can't cause an error
	if (startedPlaying && pos > 0.f) {
		startedPlaying = false;
	}

	return pos;
}

//Trigonometry calculation to figure out how high on screen a note should be
float AudioManager::getHeightOfNote(int ind, float fovy, float dist)
{
	float heightAtZero = dist * tanf((fovy / 2.f)); // horizontal height
	float height = heightAtZero * 2;
	float percentHeight = static_cast<float>(ind + 1) / (13.f);
	float noteHeight = (percentHeight - 0.5f) * height;

	return noteHeight;
}

void AudioManager::updateFrequency(double &note, double &volume) 
{	
	ALCint samplesAvailable;
	std::vector<std::complex<double>> fourierOutput;

	alcGetIntegerv(captureDev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);
	
	if (samplesAvailable < static_cast<ALCint>(size)) {
		note = 0;
		return;
	}
	
	//If the capture buffer is full enough then copy the samples over into the capture Buffer
	alcCaptureSamples(captureDev, static_cast<ALvoid*>(captureBuffer.data()), samplesAvailable);
	//Copies only a sample size number of the capture buffer (to make sure the size is always a power of 2 and consistent)
	std::vector<std::complex<double>> captureOutput(captureBuffer.data(), captureBuffer.data() + (size));
	//Copy data over from the vector to the a valarray (which is the input type of the YIN algorithm)
	std::valarray<std::complex<double>> captureOutputVal(captureOutput.data(), captureOutput.size());
	//Calculate the pitch with the YIN algorithm
	float pitch = YIN::YINalgorithm(captureOutputVal);
	
	//Change the pointer values
	note = pitch;

	//Calculate average volume
	volume = 0.f;
	for (const auto& samp : captureOutput) { volume += abs(samp); }
	volume = volume / size;
	
	return;
}