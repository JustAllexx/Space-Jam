#pragma once

#include "DrawObject.h"
#include <map>
#include <string>

//The class that handles Player Input and moving the player object around the screen
class PlayerController : public DrawObject
{
private:
	float targetY{0.f};
public:
	PlayerController();
	float posX{0.f}, posY{0.f}, velocityY, velocityX;
	void Update(float deltaTime) override;
	void controlUpdate(std::map<unsigned char, bool>& keyMap, float dt);

	int playerScore; //Score
	std::string* playerScoreText;

	void setTargetY(float target) noexcept {targetY = target;}
};