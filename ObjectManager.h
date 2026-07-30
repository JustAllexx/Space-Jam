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
public:
	//Add objects to the GUIManager Queue
	static void addObjectToQueue(DrawObject* obj);
	static void renderQueue();
	static void renderQueue(std::vector<DrawObject*> &rendQueue);
	static void Init(GLuint program);
};
