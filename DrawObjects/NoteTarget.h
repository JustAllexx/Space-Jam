#pragma once
#include "DrawObject.h"

//Forward Declarations
class AudioManager;
class PlayerController;

//The NoteBlock class
class NoteTarget : public DrawObject {
private:
	//Contains pointers to the GameManager's audio manager and the current player
	AudioManager* objAudioManager;
	//Needed to check if noteblock has collided with player
	CollisionBox colBox;
	PlayerController* currentPlayer;
	
public:
	//At what second offset should the NoteBlock be "hittable" by the player object
	float time;
	float velocity;
	static std::vector<glm::vec3> noteTargetVertices, noteTargetNormals;
	static std::vector<glm::vec2> noteTargetUvs;
	//Note Target Constructor Function
	NoteTarget(float xPos, float noteKey, float noteTime, float noteVelocity, AudioManager* inAudioManager, PlayerController* playerObject);
	void Update(float deltaTime); //Override
};