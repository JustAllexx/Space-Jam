#include "ObjectManager.h"
#include "DrawObjects/DrawObject.h"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

//glm::mat4 objModelview;
GLuint objModelviewPos, opacityPos, ambientPos, bloomPos, brightnessPos;

int newTime = 0;
int oldTIme = 0;
float deltaTime = 0.f;

ObjectManager::ObjectManager() {};

//Adds DrawObject to the render queue
void ObjectManager::addObjectToQueue(DrawObject* obj)
{
	objRenderQueue.push_back(obj);
}

//Default renderQueue function called outside the class, updates the change in time 
void ObjectManager::renderQueue() {
	glUniform1i(bloomPos, false); //Bloom should be false by default
	newTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = static_cast<float>(newTime - oldTIme) / 1000.f;
	oldTIme = newTime;

	renderQueue(objRenderQueue);
}

//Actually renders the renderQueue, called from the other function
//This type of function has a parammeter overide (has the same name as another function, but different argument requirements)
void ObjectManager::renderQueue(std::vector<DrawObject*> &rendQueue)
{
	//Iterate through all the objects in the render queue
	for (size_t objIndex = 0; objIndex < rendQueue.size(); objIndex++) {
		DrawObject* renderObj = rendQueue[objIndex];
		//Update the object
		renderObj->Update(deltaTime);
		//Calculate the modelview
		glm::mat4 model(1.f);

		model = glm::translate(model, renderObj->pos);
		model =  glm::scale(model, renderObj->scale);
		model = glm::rotate(model, renderObj->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, renderObj->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, renderObj->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

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
	for (size_t delCount = 0; delCount < rendQueue.size(); delCount++) {
		if (rendQueue[delCount]->bToDelete) {
			rendQueue.erase(rendQueue.begin() + static_cast<signed>(delCount));
		}
	}
}

void ObjectManager::Init(GLuint program)
{
	//Retrieves the locations of uniform variables inside the shader
	objModelviewPos = glGetUniformLocation(program, "modelview");
	modelView = glm::lookAt(glm::vec3(0, 0, 60.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(objModelviewPos, 1, GL_FALSE, &(modelView)[0][0]);

	opacityPos = glGetUniformLocation(program, "opacity"); //How "see-through" should an object be
	ambientPos = glGetUniformLocation(program, "ambient"); //Minimum brightness of an object
	bloomPos = glGetUniformLocation(program, "bBloom"); //Should the object have bloom
	brightnessPos = glGetUniformLocation(program, "brightness");
}