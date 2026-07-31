#include "DrawObject.h"

//The construction function for the DrawObject class
DrawObject::DrawObject(const char* modelPath, const char* texturePath, float inOpacity, float inAmbient, bool hasBloom,
	glm::vec3 inPos, glm::vec3 inScale, glm::vec3 inRotation) : texture(texturePath), mesh(modelPath),
	pos(inPos), scale(inScale), rotation(inRotation), 
	opacity(inOpacity), ambient(inAmbient), bloom(hasBloom) {
	//set Default properties
	objVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	objAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	objRotationalVelocity = glm::vec3(0.0f, 0.f, 0.f);
}

//The Default Draw Function
void DrawObject::Draw()
{
	//Binds the texture for the object into OpenGL so it can be used by the texture sampler in the fragment shader
	glBindTexture(GL_TEXTURE_2D, texture.getTextureID());

	//Bind the vertex array ID, this remembers the locations of the vertex, normals and uv buffers so we don't have to manually bind them
	GLuint vertexArrayID = mesh.getVAO();
	glBindVertexArray(vertexArrayID);

	//Draw the Object
	const std::vector<glm::vec3> vertexData = mesh.getVertices();
	int numberOfTriangles = static_cast<int>(vertexData.size());
	glDrawArrays(GL_TRIANGLES, 0, numberOfTriangles);
}

//Default setter function for private variables
void DrawObject::setRotationalVelocity(glm::vec3 newRotationalVelocity) { objRotationalVelocity = newRotationalVelocity; }
void DrawObject::setNewVelocity(glm::vec3 newVelocity) { objVelocity = newVelocity; }
void DrawObject::setNewAcceleration(glm::vec3 newAcceleration) { objAcceleration = newAcceleration; }

//Default update function called every frame (Changes the rotation and velocity if the object has a constant acceleration or velocity)
void DrawObject::Update(float deltaTime) {
	rotation += objRotationalVelocity * deltaTime;
	
	pos += objVelocity * deltaTime;
	objVelocity += objAcceleration * deltaTime;
}