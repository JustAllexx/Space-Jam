#include "ObjectLoader.h"
#include "AudioManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <vector>
#include <string>
#include <iostream>

//The class that contains information about collisions and can check if overlapping with another collision box
class CollisionBox
{
private:
public:
	float x, y, z;
	static bool checkCollision(glm::vec3 pos1, CollisionBox box1, glm::vec3 pos2, CollisionBox box2);

	CollisionBox(float inputX, float inputY, float inputZ);
	CollisionBox();

	//Used for rendering A collision box in testing, no longer needed
	GLuint vertBuffer, uvBuffer, VertexArrayID, texture;
	std::vector<glm::vec3> boxVert;
	std::vector<glm::vec2> boxVertUv;
	std::vector<glm::vec3> boxVertNorm;
};

//Class containing the base class DrawObject
class DrawObject {
private:
	//Objects velocity and acceleration
	glm::vec3 objVelocity, objAcceleration;
	glm::vec3 objRotationalVelocity;
protected:
	//Information needed for rendering the geometry of a Rendered Object, it's texture data, and geometry data
	GLuint texture;
	std::vector<glm::vec3> vertexData, normalData;
	std::vector<glm::vec2> uvData;
	GLuint vertexBuffer, uvBuffer, VertexArrayID, normalBuffer;
	//Default values for the fragment shader
	float opacity = 1.f;
	float ambient = 0.0f;
	bool bloom = false;
	float bloomAmmount = 1.f;
public:
	glm::vec3 pos, scale;
	glm::vec3 rotation;
	//Constructor Function
	DrawObject(const char* modelPath, const char* texturePath, bool bHasCollision, float inOpacity, float inAmbient, bool hasBloom,
		glm::vec3 inPos, glm::vec3 inScale, glm::vec3 inRotation, glm::vec3 collisionBoxSize);
	//Constructor should have default implementation
	DrawObject() = default;
	//Overridable draw function
	virtual void Draw();
	//set To true when the object should be deleted
	bool bToDelete = false;
	CollisionBox noteCollisionBox;
	bool hasCollision;

	//Setter functions for private variables
	void setRotationalVelocity(glm::vec3 newRotationalVelocity);
	void setNewVelocity(glm::vec3 newVelocity);
	void setNewAcceleration(glm::vec3 newAcceleration);

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
	void controlUpdate(std::map<char, bool> keyMap, float dt);
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
	NoteTarget(float xPos, float noteKey, float noteTime, float noteLength, float noteVelocity, AudioManager* inAudioManager, PlayerController* playerObject);
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


//Model Manipulation functions
//These are used to update the modelview on a draw call
class MatrixFunctions {
public:
	static glm::mat4 translate(glm::vec3 position);
	static glm::mat4 scale(glm::vec3 scale);
	static glm::mat4 rotate(glm::quat rot);

	static glm::mat4 rotateX(float angle);
	static glm::mat4 rotateY(float angle);
	static glm::mat4 rotateZ(float angle);
};

