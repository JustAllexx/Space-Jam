#include "PlayerController.h"
#include <iostream>

//Default Player Constructor Function
PlayerController::PlayerController() : DrawObject("Models/planeUV2.obj", "Textures/goldenPlane2.png", 1.f, 0.7f,
	false, glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f))
{
	//Constants about player movement
	velocityX = 20.f;
	//posX = 0.f;
	//posY = 0.f;
	velocityY = 40.f;
	targetY = 0.f;
	playerScore = 0;
	playerScoreText = nullptr;
	collisionBox.emplace(3.f, 1.5f, 6.0f);
}

//DrawObject position is updated to reflect the stored position inside the class
void PlayerController::Update([[maybe_unused]] float deltaTime) {
	pos = glm::vec3(posX, posY, 0.f);
}

//Function is called every frame
void PlayerController::controlUpdate(std::map<unsigned char, bool>& keyMap, float dt) {
	//if the user is pressing a or d, the plane will move accordingly
	//Keymap stores all the keys which are being held down by the user. keyMap['a'] returns true if a is being pressed
	if (keyMap.at('a')) {
		posX -= velocityX * dt;
	}
	else if (keyMap.at('d')) {
		posX += velocityX * dt;
	}
	
	if (abs(targetY - posY) < velocityY * dt) {
		posY = targetY;
	}
	else {
		if (targetY > posY) {
			posY += velocityY * dt;
		}
		else {
			posY -= velocityY * dt;
		}
	}
}