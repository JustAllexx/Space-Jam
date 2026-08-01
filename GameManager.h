#pragma once

#include "AudioManager.h"
#include "SceneManager.h"

#include <json/json.h>
#include <memory>
#include <numbers>
#include <string>
#include <glm/glm.hpp>

//Forward declare player controller
class PlayerController;

//Class that is responsible for controlling the game
//Is the owner of the object that plays the song the user listens to, and is the logic of the game
//Starts placing down notes for the user to try and hit
class GameManager
{
private:
	PlayerController* currentPlayer{nullptr};
	std::unique_ptr<SceneManager> sceneManager;
public:
	GameManager(std::unique_ptr<SceneManager> inSceneManager);

	void loadSongJson(const char* path, std::string& songTitle, Json::Value& notes);
	void startGame(const char* noteJsonPath, const char* noteSongPath, PlayerController* player);
	void render();
	void gameUpdate();

	//Properties about the field of view and the distance the camera is from the plane (player object)
	const float fovy{(45.f / 180.f) * static_cast<float>(std::numbers::pi)};
	const float dist{60.f};

	int score{0};
	std::string* scoreStr{nullptr};
	bool gamePlaying{false};

	AudioManager songSource;
};

