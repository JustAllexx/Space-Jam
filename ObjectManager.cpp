#include "ObjectManager.h"
#include "ObjectLoader.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

std::vector<DrawObject*> objRenderQueue;
glm::mat4 objModelview;
glm::mat4 objIdentity = glm::mat4(1.f);
std::vector <glm::mat4> objModelviewStack;
GLuint objModelviewPos, opacityPos, ambientPos, bloomPos, brightnessPos;

float objRot;
int newTime = 0;
int oldTIme = 0;
float deltaTime = 0.f;

const char* noteModelLocation = "Models/newRedCube.obj";
const char* noteTextureLocation = "Textures/newRedNote.png";

//The construction function for the DrawObject class
DrawObject::DrawObject(const char* modelPath, const char* texturePath, float inOpacity, float inAmbient, bool hasBloom,
	glm::vec3 inPos, glm::vec3 inScale, glm::vec3 inRotation) : texture(texturePath), mesh(modelPath),
	opacity(inOpacity), ambient(inAmbient), bloom(hasBloom), 
	pos(inPos), scale(inScale), rotation(inRotation) {
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
void DrawObject::Update() {
	rotation += objRotationalVelocity * deltaTime;
	
	pos += objVelocity * deltaTime;
	objVelocity += objAcceleration * deltaTime;
}

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

	objRot += 0.01f;
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
		renderObj->Update();
		//Calculate the modelview
		glm::mat4 model(1.f);

		model = glm::translate(model, renderObj->pos);
		model =  glm::scale(model, renderObj->scale);
		model = glm::rotate(model, renderObj->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, renderObj->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, renderObj->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

		//Update the vertex and fragment shader
		glUniformMatrix4fv(objModelviewPos, 1, GL_FALSE, &(objModelview * model)[0][0]);
		
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
	objModelview = glm::lookAt(glm::vec3(0, 0, 60.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(objModelviewPos, 1, GL_FALSE, &(objModelview)[0][0]);

	opacityPos = glGetUniformLocation(program, "opacity"); //How "see-through" should an object be
	ambientPos = glGetUniformLocation(program, "ambient"); //Minimum brightness of an object
	bloomPos = glGetUniformLocation(program, "bBloom"); //Should the object have bloom
	brightnessPos = glGetUniformLocation(program, "brightness");
}

//Note Target Constructor Function
NoteTarget::NoteTarget(float xPos, float noteKey, float noteTime, float noteVelocity, AudioManager* inAudioManager, PlayerController* playerObject) 
	: DrawObject(noteModelLocation, noteTextureLocation, 1.f, 0.4f,
	 true, glm::vec3(xPos, noteKey, (noteTime * -noteVelocity)),
	 glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f)),
	 objAudioManager(inAudioManager), currentPlayer(playerObject), time(noteTime), velocity(noteVelocity)
	{
	bloomAmmount = 2.5f;
	//How big object's collision should be
	collisionBox.emplace(1.f, 1.f, 1.f);
}

//Updates the note to move closer to the player as the song progresses
void NoteTarget::Update()
{
	//Calculate how close the note should be based on the play position of the song
	float playPos = objAudioManager->getPlayPos();
	float dist = time - playPos;

	glm::vec3 planePos = glm::vec3(currentPlayer->posX, currentPlayer->posY, 0);
	//Is the player object is colliding with the player, the score should be incremented and the note should be deleted
	if (CollisionBox::checkCollision(pos, colBox, planePos, currentPlayer->collisionBox.value())) {
		if (!bToDelete) {
			currentPlayer->playerScore += 100; //Increase score by 100
			*currentPlayer->playerScoreText = std::to_string(currentPlayer->playerScore);
		}
		bToDelete = true;
	}
	pos.z = (dist * -velocity) - scale.z;
	//If the note object is too far past the player it should be deleted
	if (pos.z > 40.f) {
		pos.z = 40.f;
		bToDelete = true;
	}

}

//Constructor for the note highlight
NoteHighlight::NoteHighlight(float inNoteTime, DrawObject* inParentNote, AudioManager* audioIn) :
	DrawObject("Models/noteOutline.obj", "Textures/green.png", 0.f, 1.f, false, glm::vec3(inParentNote->pos.x, inParentNote->pos.y, 0.f),
	glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 1.570796327f, 0.f))
{
	DrawObject* parentNote = inParentNote;
	pos = glm::vec3(parentNote->pos.x, parentNote->pos.y, 0.f);
	scale = glm::vec3(2.f, 2.f, 2.f);
	//Reset this line later after updating rotation
	//rotation = glm::quat(glm::vec3(0.f, 0.f, 0.f));
	rotation = glm::vec3(0.f, 1.570796327f, 0.f);
	audioManager = audioIn;
	highlightTime = 1.f;
	ambient = 1.f;
	noteTime = inNoteTime;
}

//Updates similarly to the noteObject update function, based on time
void NoteHighlight::Update()
{
	float playPos = audioManager->getPlayPos();
	float difTime = (highlightTime - (noteTime - playPos)) / highlightTime;
	//Opacity increases as the NoteObject gets closer to the player
	opacity = std::min(std::max(difTime, 0.f), 1.f);
	//Should be deleted after the point when the note should've been hit
	if (playPos > noteTime) {
		bToDelete = true;
	}
}

//Default Player Constructor Function
PlayerController::PlayerController() : DrawObject("Models/planeUV2.obj", "Textures/goldenPlane2.png", 1.f, 0.7f,
	false, glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f))
{
	//Constants about player movement
	velocityX = 20.f;
	posX = 0.f;
	posY = 0.f;
	velocityY = 40.f;
	targetY = 0.f;
	playerScore = 0;
	playerScoreText = nullptr;
}

//DrawObject position is updated to reflect the stored position inside the class
void PlayerController::Update() {
	pos = glm::vec3(posX, posY, 0.f);
}

//Setup function needed because some OpenGL calls can't be made until glut has been initialised and the program shaders have been compiled
void PlayerController::Setup()
{
	collisionBox.emplace(3.f, 1.5f, 6.0f);
}

//Function is called every frame
void PlayerController::controlUpdate(std::map<unsigned char, bool> keyMap, float dt) {
	//if the user is pressing a or d, the plane will move accordingly
	//Keymap stores all the keys which are being held down by the user. keyMap['a'] returns true if a is being pressed
	if (keyMap['a']) {
		posX -= velocityX * dt;
	}
	else if (keyMap['d']) {
		posX += velocityX * dt;
	}

	if (abs(targetY - posY) < velocityY * dt) {
		posY = targetY;
	}
	else {
		if (targetY > posY) {
			posY += velocityY * dt;
		}
		else {
			posY -= velocityY * dt;
		}
	}
}