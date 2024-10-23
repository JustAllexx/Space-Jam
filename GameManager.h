#pragma once

#include <json/json.h>
#include <fstream>
#include <iostream>
#include <string>

#include "ObjectManager.h"
#include "GUIManager.h"

//Class that is responsible for controlling the game
//Is the owner of the object that plays the song the user listens to, and is the logic of the game
//Starts placing down notes for the user to try and hit
class GameManager
{
private:
	static PlayerController* currentPlayer;
public:
	static void loadSongJson(const char* path, std::string& songTitle, Json::Value& notes);
	static void startGame(const char* noteJsonPath, const char* noteSongPath, PlayerController* player);
	static void gameUpdate();

	//Properties about the field of view and the distance the camera is from the plane (player object)
	const static float fovy;
	const static float dist;

	static int score;
	static std::string* scoreStr;
	static bool gamePlaying;

	static AudioManager songSource;

};

