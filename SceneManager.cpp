#include "SceneManager.h"
#include "DrawObjects/DrawObject.h"

#include <algorithm>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

SceneManager::SceneManager(GLuint shaderProgram) {
	objModelviewPos = glGetUniformLocation(shaderProgram, "modelview");
	modelView = glm::lookAt(glm::vec3(0, 0, 60.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(objModelviewPos, 1, GL_FALSE, &(modelView)[0][0]);

	opacityPos = glGetUniformLocation(shaderProgram, "opacity"); //How "see-through" should an object be
	ambientPos = glGetUniformLocation(shaderProgram, "ambient"); //Minimum brightness of an object
	bloomPos = glGetUniformLocation(shaderProgram, "bBloom"); //Should the object have bloom
	brightnessPos = glGetUniformLocation(shaderProgram, "brightness");
};

//Adds DrawObject to the render queue
void SceneManager::addObjectToQueue(DrawObject* obj)
{
	objRenderQueue.push_back(obj);
}

//Default renderQueue function called outside the class, updates the change in time 
void SceneManager::renderQueue() {
	glUniform1i(bloomPos, false); //Bloom should be false by default
	currentFrameTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = static_cast<float>(currentFrameTime - lastFrameTime) / 1000.f;
	lastFrameTime = currentFrameTime;

	renderQueue(objRenderQueue);
}

//Actually renders the renderQueue, called from the other function
//This type of function has a parammeter overide (has the same name as another function, but different argument requirements)
void SceneManager::renderQueue(std::vector<DrawObject*> &rendQueue)
{
	//Iterate through all the objects in the render queue
	for (size_t objIndex = 0; objIndex < rendQueue.size(); objIndex++) {
		DrawObject* renderObj = rendQueue[objIndex];
		//Update the object
		renderObj->Update(deltaTime);
		//Calculate the modelview
		glm::mat4 model(1.f);

		model = glm::translate(model, renderObj->getPosition());
		model =  glm::scale(model, renderObj->getScale());
		glm::vec3 rotation = renderObj->getRotation();
		model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

		//Update the vertex and fragment shader
		glUniformMatrix4fv(objModelviewPos, 1, GL_FALSE, &(modelView * model)[0][0]);
		
		//Update the fragment shader with these details
		glUniform1f(opacityPos, renderObj->getOpacity());
		glUniform1f(ambientPos, renderObj->getAmbient());
		glUniform1i(bloomPos, renderObj->isBloom());
		glUniform1f(brightnessPos, renderObj->getBloomAmmount());
		
		//Call that object's draw function
		renderObj->Draw();
	}
	//Delete all objects flagged for deletion
	rendQueue.erase(
    std::remove_if(rendQueue.begin(), rendQueue.end(),
        [](const auto& obj) {
            return obj->bToDelete;
        }),
    rendQueue.end());
}