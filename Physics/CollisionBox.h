#pragma once

//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <vector>

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