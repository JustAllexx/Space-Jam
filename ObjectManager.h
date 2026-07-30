#include "ObjectLoader.h"
#include "AudioManager.h"
#include "Physics/CollisionBox.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <optional>
#include <vector>
#include <string>
#include <map>

//Class containing the base class DrawObject
class DrawObject {
private:
	//Objects velocity and acceleration
	glm::vec3 objVelocity, objAcceleration;
	glm::vec3 objRotationalVelocity;
protected:
	//Information needed for rendering the geometry of a Rendered Object, it's texture data, and geometry data
	Texture texture;
	Mesh mesh;

	//Default values for the fragment shader
	float opacity = 1.f;
	float ambient = 0.0f;
	bool bloom = false;
	float bloomAmmount = 1.f;
public:
	glm::vec3 pos, scale;
	glm::vec3 rotation;
	//Constructor Function
	DrawObject(const char* modelPath, const char* texturePath, float inOpacity, float inAmbient, bool hasBloom,
		glm::vec3 inPos, glm::vec3 inScale, glm::vec3 inRotation);

	//set To true when the object should be deleted
	bool bToDelete = false;
	std::optional<CollisionBox> noteCollisionBox;

	//Setter functions for private variables
	void setRotationalVelocity(glm::vec3 newRotationalVelocity);
	void setNewVelocity(glm::vec3 newVelocity);
	void setNewAcceleration(glm::vec3 newAcceleration);
	
	//Overridable draw and update functions
	virtual void Draw();
	virtual void Update();
};

#pragma once
//Object Manager Class
class ObjectManager
{
public:
	//Add objects to the GUIManager Queue
	static void addObjectToQueue(DrawObject* obj);
	static void renderQueue();
	static void renderQueue(std::vector<DrawObject*> &rendQueue);
	static void Init(GLuint program);
};

//The class that handles Player Input and moving the player object around the screen
class PlayerController : public DrawObject
{
public:
	PlayerController();
	float posX, posY, targetY, velocityY, velocityX;
	void Setup(const char* tPath, GLuint planeShaderProgram);
	void Update();
	void controlUpdate(std::map<unsigned char, bool> keyMap, float dt);
	//The player's collision
	CollisionBox playerCollision;

	int playerScore; //Score
	std::string* playerScoreText;
};

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
	void Update(); //Override
};

//Highlights used in game, these show up on screen when the noteblock is close to being "hittable"
class NoteHighlight : public DrawObject {
private:
	AudioManager* audioManager;
	float highlightTime;
	float noteTime;
public:
	NoteHighlight(float inNoteTime, DrawObject* inParentNote, AudioManager* audioIn);
	void Update();
};
