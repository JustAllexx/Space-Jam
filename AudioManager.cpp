#include "AudioManager.h"

#include <iostream>

const ALCuint rate = 44100;
const ALCuint size = 1024;
const std::vector<std::string> notes = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
std::map<std::string, int> noteIndexes;

double pi = 2 * acos(0.0);
std::complex<double> posi = std::complex<double>(0.f, 1.f);

//The buffer that is written to when capturing microphone input
INT16 CaptureBuffer[22050];
float deltaCheck;


AudioManager::AudioManager() {
	//Connect to the two audio devices (connects to the microphone and connects to the speakers)
	setupDevice();
	//Create an OpenAL source that can start playing audio
	setupSource();
}

//Creates an OpenAl source with specific properties
void AudioManager::setupSource() {
	alGenSources(1, &source);
	alSourcef(source, AL_PITCH, 1.f);
	alSourcef(source, AL_GAIN, 1.f);
	alSource3f(source, AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(source, AL_VELOCITY, 0.f, 0.f, 0.f);
	alSourcei(source, AL_LOOPING, false);
	//The OpenAL location of the audio being played
	alSourcei(source, AL_BUFFER, playingBuffer);
}

void AudioManager::setupDevice() {
	const ALCchar* devices;
	
	device = alcOpenDevice(nullptr);
	if (!device) { std::cout << "Couldn't open sound device" << std::endl; return; }

	context = alcCreateContext(device, nullptr);
	if (!context) { std::cout << "Failed to create context" << std::endl; return; }

	alcMakeContextCurrent(context);
	devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	//checks that a device was found
	if (devices == NULL) {
		std::cout << "Input device not found" << std::endl;
		return;
	}
	//Opens the capture device in "Mono" format, this means that only one integer will be recorder per sample (and it will be of type integer 16 or 16 bits).
	captureDev = alcCaptureOpenDevice(NULL, rate, AL_FORMAT_MONO16, size);

	if (captureDev == NULL) {
		printf("Failed to open Input Device");
	}
}

//Play audio
void AudioManager::playAudioBuffer(ALuint buffer) {
	//Buffers the audio content into the source
	alSourcei(source, AL_BUFFER, buffer);
	//Starts playing
	alSourcePlay(source);
	startedPlaying = true;
}

ALuint AudioManager::addAudioBuffer(const char* path) {
	SF_INFO info;

	SNDFILE* soundFile = sf_open(path, SFM_READ, &info);
	if (!soundFile) { 
		std::cout << "Failed to open soundfile" << std::endl; 
		sf_close(soundFile);
		return 0;
	}

	//Find out what format the audio is in
	ALenum format = AL_NONE;
	if (info.channels == 1) { format = AL_FORMAT_MONO16; }
	else if (info.channels == 2) { format = AL_FORMAT_STEREO16; }
	else { 
		std::cout << "Incorrect Format: more than 2 channels" << std::endl; 
		sf_close(soundFile);
		return 0; 
	}

	//How big the filesize is going to be in bytes
	int fileSize = info.frames * info.channels * sizeof(short);
	//Allocate that much memory to a short pointer
	short* memory = static_cast<short*>(malloc(fileSize));
	//Buffer in the soundfile to that memory pointer
	sf_readf_short(soundFile, memory, info.frames);

	//Buffer it into an openAl buffer which can then be played by a source
	ALuint buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, memory, fileSize, info.samplerate);
	//Delete the memory taken up by the memoryPointer and close the sound file
	free(memory);
	sf_close(soundFile);

	audioBuffers.push_back(buffer);
	return buffer;
}

//Starts capturing audio using the capture device
void AudioManager::StartCapture() {
	alcCaptureStart(captureDev);
	deltaCheck = 0.f;
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
	float heightAtZero = dist * tan((fovy / 2.f)); // horizontal height
	float height = heightAtZero * 2;
	float percentHeight = (ind + 1) / (13.f);
	float noteHeight = (percentHeight - 0.5f) * height;

	return noteHeight;
}

void AudioManager::updateFrequency(float dt, double &note, double &volume) 
{

	deltaCheck += dt;
	
	ALint samplesAvailable;
	std::vector<std::complex<double>> fourierOutput;
	double max_mag = 0;
	int mag_i = 0;

	alcGetIntegerv(captureDev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);
	
	if (samplesAvailable < size) {
		note = 0;
		return;
	}
	
	deltaCheck = 0.f;
	//If the capture buffer is full enough then copy the samples over into the capture Buffer
	alcCaptureSamples(captureDev, (ALvoid*)CaptureBuffer, samplesAvailable);
	//Copies only a sample size number of the capture buffer (to make sure the size is always a power of 2 and consistent)
	std::vector<std::complex<double>> captureOutput(CaptureBuffer, CaptureBuffer + (size));
	//Copy data over from the vector to the a valarray (which is the input type of the YIN algorithm)
	std::valarray<std::complex<double>> captureOutputVal(captureOutput.data(), captureOutput.size());
	//Calculate the pitch with the YIN algorithm
	float pitch = YIN::YINalgorithm(captureOutputVal);
	
	//Change the pointer values
	note = pitch;

	//Calculate average volume
	volume = 0.f;
	for (std::complex<double> samp : captureOutput) { volume += abs(samp); }
	volume = volume / size;
	
	return;

}

