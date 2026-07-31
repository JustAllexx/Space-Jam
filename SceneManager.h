#pragma once

#include <GL/glew.h>
//#include <GL/glut.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

//Forward Declarations
class DrawObject;

//Object Manager Class
class SceneManager
{
private:
	std::vector<DrawObject*> objRenderQueue;
	std::vector<glm::mat4> modelViewStack;
	glm::mat4 modelView;
	GLuint objModelviewPos, opacityPos, ambientPos, bloomPos, brightnessPos;
	//For calculating deltaTime
	int lastFrameTime{0};
	int currentFrameTime{0};
	float deltaTime{0.f};
public:
	SceneManager(GLuint shaderProgram);

	//Add objects to the GUIManager Queue
	void addObjectToQueue(DrawObject* obj);
	void renderQueue();
	void renderQueue(std::vector<DrawObject*> &rendQueue);

	float getDeltaTime() const noexcept {return deltaTime;}
};
