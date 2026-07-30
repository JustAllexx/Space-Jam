#include "CollisionBox.h"

//CollisionBox constructor function (no arguments for default collision box which has all sides size 1)
CollisionBox::CollisionBox() {
	x = 1.f; y = 1.f; z = 1.f;
}

CollisionBox::CollisionBox(float width, float height, float length) {
	x = width;
	y = height;
	z = length;
}

//This implementation assumes the collision box will never rotate
//Takes two collision boxes and their positions as inputs and returns true if they are overlapping, false if they are not
bool CollisionBox::checkCollision(glm::vec3 pos1, CollisionBox box1, glm::vec3 pos2, CollisionBox box2) {

	bool collisionX = pos1.x - box1.x <= pos2.x + box2.x && pos1.x + box1.x >= pos2.x - box2.x;
	bool collisionY = pos1.y - box1.y <= pos2.y + box2.y && pos1.y + box1.y >= pos2.y - box2.y;
	bool collisionZ = pos1.z - box1.z <= pos2.z + box2.z && pos1.z + box1.z >= pos2.z - box2.z;

	//std::cout << "X: " << collisionX << " Y: " << collisionY << " Z: " << collisionZ << std::endl;
	return collisionX && collisionY && collisionZ;
}