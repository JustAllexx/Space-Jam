#include "GameManager.h"
#include "AudioManager.h"
#include "GUIManager.h"
#include "SceneManager.h"
#include "DrawObjects/PlayerController.h"
#include "DrawObjects/NoteTarget.h"
#include "DrawObjects/NoteHighlight.h"
#include "GUIObjects/GUIButton.h"
#include "Utilities/ShaderLoader.h"

#include "json/forwards.h"
#include <OptionsManager.h>
#include <fstream>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <string>

//The index of each note counting up from 0
const std::map<std::string, int> notePairings{
	{"A", 0},
	{"A#", 1},
	{"B", 2},
	{"C", 3},
	{"C#", 4},
	{"D", 5},
	{"D#", 6},
	{"E", 7},
	{"F", 8},
	{"F#", 9},
	{"G", 10},
	{"G#", 11}
};

GameManager::GameManager(Program& shaderProgram, Program& guiProgram, std::map<unsigned char, bool>& inKeyMap) 
: guiManager(std::make_unique<GUIManager>(guiProgram)),
  currentPlayer(std::make_unique<PlayerController>()), 
  sceneManager(std::make_unique<SceneManager>(shaderProgram)),
  audioManager(std::make_unique<AudioManager>()), 
  optionsManager(std::make_unique<OptionsManager>(*guiManager, *this)),
  keyMap(inKeyMap) 
{
	sceneManager->addObjectToQueue(currentPlayer.get());
}

//The function that loads the song file
void GameManager::loadSongJson(const char* path, std::string& songTitle, Json::Value& notes)
{
	std::ifstream jsonFile(path);
	Json::Value root;
	jsonFile >> root;

	songTitle = root["Song Title"].asCString();
	notes = root["Notes"];	
}

//This function is called in the Main Module to start the game, takes in 3 parameters: the path of the Songs Notes in Json form, the path of the sound file to play over the song, and the player class
void GameManager::startGame(const char* noteJsonPath, const char* noteSongPath)
{
	std::string songTitle;
	Json::Value notes;
	loadSongJson(noteJsonPath, songTitle, notes);
	//currentPlayer = player;
	//Resets the players score to 0
	currentPlayer->playerScore = 0;
	//*currentPlayer->playerScoreText = "0";

	gamePlaying = true;

	//audioManager = AudioManager();
	// IMPORTANT: For now the audio identifier will be the sound path, this will change later
	audioManager->addAudioBuffer(noteSongPath, noteSongPath);

	Json::ArrayIndex x = notes.size();
	//This loop places down all the notes in a file into the object manager queues so that they can be rendered and sent towards the player
	for (Json::ArrayIndex i = 0; i < x; i++) {
		//Extracts 2 values about each note, it's value (to calculate how high on the screen it should be, and what time it should be played at)
		std::string noteValue = notes[i][0].asCString();
		float time = notes[i][1].asFloat();
		int noteIndex = notePairings.at(noteValue);

		float height = AudioManager::getHeightOfNote(noteIndex, fovy, dist);

		DrawObject* noteObject = new NoteTarget(0.f, height, time, 40.f, audioManager.get(), currentPlayer.get());
		DrawObject* noteHighlight = new NoteHighlight(time, noteObject, audioManager.get());
		sceneManager->addObjectToQueue(noteObject);
		sceneManager->addObjectToQueue(noteHighlight);
	}
	//Finally plays the song
	audioManager->playAudioBuffer(noteSongPath);
	guiManager->showGameGUI();
}

void GameManager::render() {
	sceneManager->renderQueue();
	guiManager->renderQueue();
}

//Called every frame
void GameManager::gameUpdate()
{
	//First update GUI
	if (clickPosition.has_value()) {
		guiManager->checkCollisions(clickPosition->x, clickPosition->y, true);
		clickPosition.reset();
	}
	guiManager->checkCollisions(mousePosition.x, mousePosition.y, false);

	double note, volume;
	note = 0.f;
	volume = 0.f;
	//If the audio manager returns 0, that means that no new frequency can be calculated because the capture buffer isn't filled yet
	//Or that the frequency calculated did not dip below the harmony threshold, so couldn't return an accurate value
	//This function also returns a volume, if the average volume (or gain) of the capture buffer was not above 400.f, then we ignore the value because the capture taken was too quiet
	audioManager->updateFrequency(note, volume);
	
	if (note != 0 && volume > 400) {
		//Equation for calculating the piano key value of a frequency
		double key = (12 * log2(note / 440.f) + 49);
		//Can use this to determine the note was being sung
		key = std::fmod(key, 12);
		int keyInd = static_cast<int>(std::round(key));
		//this is passed on to a static function that calculates the height that the player should be on screen based on the value of the note sung
		float targetY = AudioManager::getHeightOfNote(keyInd, fovy, dist);
		currentPlayer->setTargetY(targetY);
	}
	//Update the players movement
	//Keymap contains what keys are being pressed down during this frame, dt is the time since last frame
	currentPlayer->controlUpdate(keyMap, sceneManager->getDeltaTime());
	guiManager->GameGUI_ScoreText->text = std::to_string(currentPlayer->playerScore);

	glm::vec3 playerLightPos(currentPlayer->posX, currentPlayer->posY + 2.f, 0.f);
	Program& shaderProgram = sceneManager->getShaderProgram();
	shaderProgram.setVec3("lightPos", playerLightPos);

	float currentPlayPosition =  audioManager->getPlayPos();
	//The code that checks if the game should finish
	if (gamePlaying == true && currentPlayPosition == 0 && audioManager->isPlaying() == false) {
		gamePlaying = false;
		//If the game is finished update the score screen and direct the player to it
		guiManager->scoreScreen_FinalScoreText->text = std::to_string(currentPlayer->playerScore);
		guiManager->showScoreMenu();
	}
}

void GameManager::Init() {
	sceneManager->createNightSky();
	guiManager->showMainMenu();
	//Obviously, song playing and audio recording concerns will be seperated at a later point
	audioManager->StartCapture();
}

