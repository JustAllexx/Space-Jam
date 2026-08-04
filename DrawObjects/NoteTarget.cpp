#include "NoteTarget.h"
#include "../AudioManager.h"
#include "PlayerController.h"

const char* noteModelLocation = "Models/newRedCube.obj";
const char* noteTextureLocation = "Textures/newRedNote.png";

//Note Target Constructor Function
NoteTarget::NoteTarget(float xPos, float noteKey, float noteTime, float noteVelocity, AudioManager* inAudioManager, PlayerController* playerObject) 
	: DrawObject(noteModelLocation, noteTextureLocation, 1.f, 0.4f,
	 true, glm::vec3(xPos, noteKey, (noteTime * -noteVelocity)),
	 glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f)),
	 objAudioManager(inAudioManager), currentPlayer(playerObject), time(noteTime), velocity(noteVelocity)
	{
	bloomAmmount = 2.5f;
	//How big object's collision should be
	collisionBox.emplace(1.f, 1.f, 1.f);
}

//Updates the note to move closer to the player as the song progresses
void NoteTarget::Update([[maybe_unused]] float deltaTime)
{
	//Calculate how close the note should be based on the play position of the song
	float playPos = objAudioManager->getPlayPos();
	float dist = time - playPos;

	glm::vec3 planePos = glm::vec3(currentPlayer->posX, currentPlayer->posY, 0);
	//Is the player object is colliding with the player, the score should be incremented and the note should be deleted
	if (CollisionBox::checkCollision(pos, colBox, planePos, currentPlayer->collisionBox.value())) {
		if (!bToDelete) {
			currentPlayer->playerScore += 100; //Increase score by 100
			//*currentPlayer->playerScoreText = std::to_string(currentPlayer->playerScore);
		}
		bToDelete = true;
	}
	pos.z = (dist * -velocity) - scale.z;
	//If the note object is too far past the player it should be deleted
	if (pos.z > 40.f) {
		pos.z = 40.f;
		bToDelete = true;
	}

}