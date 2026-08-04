#pragma once

#include "Utilities/ObjectLoader.h"
#include "../Physics/CollisionBox.h"

#include <optional>

//Class containing the base class DrawObject
class DrawObject {
protected:
	//Information needed for rendering the geometry of a Rendered Object, it's texture data, and geometry data
	Texture texture;
	Mesh mesh;

	//Transform information
	glm::vec3 pos, scale;
	glm::vec3 rotation;
	//Physics information
	glm::vec3 objVelocity, objAcceleration;
	glm::vec3 objRotationalVelocity;

	//Default values for the fragment shader
	float opacity{1.f};
	float ambient{0.0f};
	bool bloom{false};
	float bloomAmmount{1.f};

	//set To true when the object should be deleted
	bool bToDelete{false};
public:
	//Constructor Function
	DrawObject(const char* modelPath, const char* texturePath, float inOpacity, float inAmbient, bool hasBloom,
		glm::vec3 inPos, glm::vec3 inScale, glm::vec3 inRotation);

	//set To true when the object should be deleted
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

	glm::vec3 getPosition() const noexcept {return pos;}
	glm::vec3 getScale() const noexcept {return scale;}
	glm::vec3 getRotation() const noexcept {return rotation;}
	bool IsDestroy() const noexcept {return bToDelete;}

	void Destroy() noexcept {bToDelete = true;}
	
	//Overridable draw and update functions
	virtual void Draw();
	virtual void Update(float deltaTime);
};