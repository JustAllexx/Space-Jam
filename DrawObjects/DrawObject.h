#pragma once

#include "../ObjectLoader.h"
#include "../Physics/CollisionBox.h"

#include <optional>

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
	std::optional<CollisionBox> collisionBox;

	//Setter functions for private variables
	void setRotationalVelocity(glm::vec3 newRotationalVelocity);
	void setNewVelocity(glm::vec3 newVelocity);
	void setNewAcceleration(glm::vec3 newAcceleration);
	
	//Getters for fragment shader
	float getOpacity() const noexcept {return opacity;}
	float getAmbient() const noexcept {return ambient;}
	bool isBloom() const noexcept {return bloom;}
	float getBloomAmmount() const noexcept {return bloomAmmount;}
	
	//Overridable draw and update functions
	virtual void Draw();
	virtual void Update(float deltaTime);
};