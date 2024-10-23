#include "GameManager.h"

//Class variables defined out of scope
AudioManager GameManager::songSource;
int GameManager::score = 0;
PlayerController* GameManager::currentPlayer = nullptr;
std::string* GameManager::scoreStr = nullptr;
bool GameManager::gamePlaying = false;

const float GameManager::fovy = (45.f / 180.f) * glm::pi<float>();
const float GameManager::dist = 60.f;

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
void GameManager::startGame(const char* noteJsonPath, const char* noteSongPath, PlayerController* player)
{
	std::string songTitle;
	Json::Value notes;
	loadSongJson(noteJsonPath, songTitle, notes);
	currentPlayer = player;
	//Resets the players score to 0
	currentPlayer->playerScore = 0;
	*currentPlayer->playerScoreText = "0";

	gamePlaying = true;

	songSource = AudioManager();
	ALuint songBuffer = songSource.addAudioBuffer(noteSongPath);

	int x = notes.size();
	//This loop places down all the notes in a file into the object manager queues so that they can be rendered and sent towards the player
	for (int i = 0; i < x; i++) {
		//Extracts 2 values about each note, it's value (to calculate how high on the screen it should be, and what time it should be played at)
		std::string noteValue = notes[i][0].asCString();
		float time = notes[i][1].asFloat();
		int noteIndex = notePairings.at(noteValue);

		float height = AudioManager::getHeightOfNote(noteIndex, fovy, dist);

		DrawObject* noteObject = new NoteTarget(0.f, height, time, 1.f, 40.f, &songSource, player);
		ObjectManager::addObjectToQueue(noteObject);
	}
	//Finally plays the song
	songSource.playAudioBuffer(songBuffer);
}

//Called every frame
void GameManager::gameUpdate()
{
	float currentPlayPosition =  songSource.getPlayPos();
	//The code that checks if the game should finish
	if (gamePlaying == true && currentPlayPosition == 0 && songSource.startedPlaying == false) {
		gamePlaying = false;
		//If the game is finished update the score screen and direct the player to it
		GUIManager::scoreScreen_FinalScoreText->text = std::to_string(currentPlayer->playerScore);
		GUIManager::showScoreMenu();
	}
}


