#pragma once

#include <GL/glew.h>
//#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

//Forward Declarations
class DrawObject;

//Object Manager Class
class ObjectManager
{
private:
	std::vector<DrawObject*> objRenderQueue;
	std::vector<glm::mat4> modelViewStack;
public:
	ObjectManager();

	//Add objects to the GUIManager Queue
	void addObjectToQueue(DrawObject* obj);
	void renderQueue();
	void renderQueue(std::vector<DrawObject*> &rendQueue);
	void Init(GLuint program);
};
