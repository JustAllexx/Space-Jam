#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/fwd.hpp>
#include <vector>

//Forward Declarations
class DrawObject;
class Program;

//Object Manager Class
class SceneManager
{
private:
	std::vector<DrawObject*> objRenderQueue;
	std::vector<glm::mat4> modelViewStack;
	glm::mat4 modelView;
	Program& shaderProgram;
	//For calculating deltaTime
	int lastFrameTime{0};
	int currentFrameTime{0};
	float deltaTime{0.f};
public:
	SceneManager(Program& shaderProgram);
	~SceneManager() = default;

	//Add objects to the GUIManager Queue
	void addObjectToQueue(DrawObject* obj);
	void renderQueue();
	void renderQueue(std::vector<DrawObject*> &rendQueue);
	//Most likely temporary creation function
	void createNightSky();

	float getDeltaTime() const noexcept {return deltaTime;}
	Program& getShaderProgram() const noexcept {return shaderProgram;}
};
