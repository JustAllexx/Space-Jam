#pragma once

#include "AudioManager.h"
#include "GUIManager.h"
#include "SceneManager.h"

#include <glm/ext/vector_int2.hpp>
#include <json/json.h>
#include <memory>
#include <numbers>
#include <optional>
#include <string>
#include <glm/glm.hpp>
#include <map>

//Forward declare player controller
class PlayerController;
class Program;

//Class that is responsible for controlling the game
//Is the owner of the object that plays the song the user listens to, and is the logic of the game
//Starts placing down notes for the user to try and hit
class GameManager
{
private:
	GUIManager* guiManager{nullptr};
	std::unique_ptr<PlayerController> currentPlayer;
	std::unique_ptr<SceneManager> sceneManager;
	std::unique_ptr<AudioManager> audioManager;
	//Input store
	std::map<unsigned char, bool>& keyMap;
	glm::ivec2 mousePosition{0, 0};
	std::optional<glm::ivec2> clickPosition;
public:
	GameManager(Program& shaderProgram, std::map<unsigned char, bool>& inKeyMap, GUIManager* inGUIManager);
	
	//The only class allowed to have an init, as creation of the game object and presenting the title screen should be seperate
	void Init();
	void loadSongJson(const char* path, std::string& songTitle, Json::Value& notes);
	void startGame(const char* noteJsonPath, const char* noteSongPath);
	void render();
	void gameUpdate();
	void setMousePosition(glm::ivec2 mousePosition_) noexcept {mousePosition = mousePosition_;}
	void setMouseClicked(glm::ivec2 mouseClicked) noexcept {
		if (!clickPosition.has_value()) {
			clickPosition.emplace(mouseClicked);
		}
	}

	//Properties about the field of view and the distance the camera is from the plane (player object)
	const float fovy{(45.f / 180.f) * static_cast<float>(std::numbers::pi)};
	const float dist{60.f};

	int score{0};
	std::string* scoreStr{nullptr};
	bool gamePlaying{false};

	GUIManager& getGUIManager() const noexcept {return *guiManager;}
};

