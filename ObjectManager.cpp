#include "ObjectManager.h"

std::vector<DrawObject*> objRenderQueue;
glm::mat4 objModelview;
glm::mat4 objIdentity = glm::mat4(1.f);
std::vector <glm::mat4> objModelviewStack;
GLuint objModelviewPos, opacityPos, ambientPos, bloomPos, brightnessPos;

std::map<const char*, GLuint> textureLoaderMap;

float objRot;
float newTime = 0.f;
float oldTIme = 0.f;
float deltaTime = 0.f;

const char* noteModelLocation = "Models\\newRedCube.obj";
const char* noteTextureLocation = "Textures\\newRedNote.png";

//The construction function for the DrawObject class
DrawObject::DrawObject(const char* modelPath, const char* texturePath, bool bHasCollision, float inOpacity, float inAmbient, bool hasBloom,
	glm::vec3 inPos, glm::vec3 inScale, glm::vec3 inRotation, glm::vec3 collisionBoxSize)
{
	//Set class properties
	hasCollision = bHasCollision;
	pos = inPos;
	scale = inScale;
	rotation = inRotation;
	opacity = inOpacity;
	ambient = inAmbient;
	bloom = hasBloom;
	//set Default properties
	objVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	objAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	objRotationalVelocity = glm::vec3(0.0f, 0.f, 0.f);
	//If the texture is already loaded it doesn't need to be loaded again
	//Texture loader map keeps track of the paths of textures that have been buffered into OpenGL
	if (textureLoaderMap[texturePath] == 0) {
		texture = ObjectLoader::loadTexture(texturePath);
		textureLoaderMap[texturePath] = texture;
	}
	else {
		texture = textureLoaderMap[texturePath];
	}
	//Loads the wavefront file with the obj loader and returns the details into these 3 array: vertexData, uvData, normalData
	ObjectLoader::loadOBJ(modelPath, vertexData, uvData, normalData);
	
	//Buffers all the vertex and uv and normal data into their own buffers, these buffers store the data so it can be binded and then rendered
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::vec3), &vertexData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof(glm::vec2), &uvData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(glm::vec3), &normalData[0], GL_STATIC_DRAW);

	objRenderQueue.push_back(this);
}

//The Default Draw Function
void DrawObject::Draw()
{
	//Update the fragment shader with these details
	glUniform1f(opacityPos, opacity);
	glUniform1f(ambientPos, ambient);
	glUniform1i(bloomPos, bloom);
	glUniform1f(brightnessPos, bloomAmmount);

	//Binds the texture for the object into OpenGL so it can be used by the texture sampler in the fragment shader
	glBindTexture(GL_TEXTURE_2D, texture);
	//Load all the buffers into OpenGL so they can be used in the vertex and fragment shaders
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer(1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	
	glVertexAttribPointer(2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	//Draw the Object
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size());
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
	glUniform1i(bloomPos, false); //Bloom should be false by defauly
	newTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = (newTime - oldTIme) / 1000.f;
	oldTIme = newTime;

	objRot += 0.01f;
	renderQueue(objRenderQueue);
}

//Actually renders the renderQueue, called from the other function
//This type of function has a parammeter overide (has the same name as another function, but different argument requirements)
void ObjectManager::renderQueue(std::vector<DrawObject*> &rendQueue)
{
	//Iterate through all the objects in the render queue
	for (int objIndex = 0; objIndex < rendQueue.size(); objIndex++) {
		DrawObject* renderObj = rendQueue[objIndex];
		//Update the object
		renderObj->Update();
		//Calculate the modelview
		glm::mat4 translateMatrix = MatrixFunctions::translate(renderObj->pos);
		glm::mat4 scaleMatrix = MatrixFunctions::scale(renderObj->scale);

		glm::vec3 vec3rot = renderObj->rotation;

		glm::mat4 rotMatrix = MatrixFunctions::rotateZ(vec3rot.z) * MatrixFunctions::rotateY(vec3rot.y) * MatrixFunctions::rotateX(vec3rot.x);

		glm::mat4 transformMat = translateMatrix * rotMatrix * scaleMatrix;
		//Update the vertex and fragment shader
		glUniformMatrix4fv(objModelviewPos, 1, GL_FALSE, &(objModelview * transformMat)[0][0]);
		//Call that object's draw function
		renderObj->Draw();
	}
	//Delete all objects flagged for deletion
	for (int delCount = 0; delCount < rendQueue.size(); delCount++) {
		if (rendQueue[delCount]->bToDelete) {
			rendQueue.erase(rendQueue.begin() + delCount);
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
NoteTarget::NoteTarget(float xPos, float noteKey, float noteTime, float noteLength, float noteVelocity, AudioManager* inAudioManager, PlayerController* playerObject)
{
	//DrawObject inherited properties defined for this class
	hasCollision = true;
	time = noteTime;
	pos = glm::vec3(xPos, noteKey, (noteTime * -noteVelocity));
	scale = glm::vec3(2.f, 2.f, 2.f);
	rotation = glm::vec3(0.f, 0.f, 0.f);
	bloom = true;
	objAudioManager = inAudioManager;
	currentPlayer = playerObject;
	ambient = 0.4f;
	bloomAmmount = 2.5f;
	//How big object's collision should be
	noteCollisionBox = CollisionBox(1.f, 1.f, 1.f);

	velocity = noteVelocity;

	//Same code to buffer vertex, normal and uv data as in the default constructor in the DrawObject class, does the same thing here
	if (textureLoaderMap[noteTextureLocation] == 0) {
		texture = ObjectLoader::loadTexture(noteTextureLocation);
	}
	else {
		texture = textureLoaderMap[noteTextureLocation];
	}
	ObjectLoader::loadOBJ(noteModelLocation, vertexData, uvData, normalData);

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::vec3), &vertexData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof(glm::vec2), &uvData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(glm::vec3), &normalData[0], GL_STATIC_DRAW);

	//Calls for the creation of a NoteHighlight, a note highlight outlines on the screen where a note is going to be
	DrawObject* noteHighlight = new NoteHighlight(noteTime, this, inAudioManager);
	objRenderQueue.push_back(this);
}

//Updates the note to move closer to the player as the song progresses
void NoteTarget::Update()
{
	//Calculate how close the note should be based on the play position of the song
	float playPos = objAudioManager->getPlayPos();
	float dist = time - playPos;

	glm::vec3 planePos = glm::vec3(currentPlayer->posX, currentPlayer->posY, 0);
	//Is the player object is colliding with the player, the score should be incremented and the note should be deleted
	if (CollisionBox::checkCollision(pos, colBox, planePos, currentPlayer->playerCollision)) {
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
NoteHighlight::NoteHighlight(float inNoteTime, DrawObject* inParentNote, AudioManager* audioIn)
{
	hasCollision = false;
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

	texture = ObjectLoader::loadTexture("Textures\\green.png");
	ObjectLoader::loadOBJ("Models\\noteOutline.obj", vertexData, uvData, normalData);
	//Same code for buffering in the vertex, uv and normal data as the DrawObject class
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::vec3), &vertexData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof(glm::vec2), &uvData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(glm::vec3), &normalData[0], GL_STATIC_DRAW);

	objRenderQueue.push_back(this);

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

const char* filePath = "Models\\plane.obj";
const char* texturePath;
GLuint planeOpacityPos, planeAmbientPos;

//Default Player Constructor Function
PlayerController::PlayerController()
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
void PlayerController::Setup(const char* tPath, GLuint planeShaderProgram)
{
	scale = glm::vec3(1.f, 1.f, 1.f);
	opacity = 1.f;
	ambient = 0.7f;

	texturePath = tPath;

	planeOpacityPos = glGetUniformLocation(planeShaderProgram, "opacity");
	planeAmbientPos = glGetUniformLocation(planeShaderProgram, "ambient");

	playerCollision = CollisionBox(3.f, 1.5f, 6.0f);
	//Loads in vertex, normal and uv data; same as the default draw object function
	ObjectLoader::loadOBJ("Models\\planeUV2.obj", vertexData, uvData, normalData);
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::vec3), &vertexData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof(glm::vec2), &uvData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(glm::vec3), &normalData[0], GL_STATIC_DRAW);

	texture = ObjectLoader::loadTexture(tPath);
	ObjectManager::addObjectToQueue(this);
}

//Function is called every frame
void PlayerController::controlUpdate(std::map<char, bool> keyMap, float dt) {
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

//Matrix Functions
glm::mat4 MatrixFunctions::translate(glm::vec3 position) {
	glm::mat4 transMat =  glm::mat4(
		1, 0, 0, position.x,
		0, 1, 0, position.y,
		0, 0, 1, position.z,
		0, 0, 0, 1
	);
	
	//The way GLM defines it's parameter order when called mat4 means every matrix has to be transposed before it can be used
	return glm::transpose(transMat);
}

glm::mat4 MatrixFunctions::scale(glm::vec3 scale) {
	glm::mat4 scaleMat = glm::mat4(
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
	);
	return glm::transpose(scaleMat);
}

glm::mat4 MatrixFunctions::rotate(glm::quat rot) {
	
	float a = rot.x;
	float b = rot.y;
	float c = rot.z;
	float d = rot.w;
	float a2 = a * a; float b2 = b * b; float c2 = c * c; float d2 = d * d;
	
	glm::mat4 rotMat = glm::mat4(
		(2.f * (a2 + b2)) - 1.f, 2.f * ((b * c) - (a * d)), 2.f * ((b * d) + (a * c)), 0,

		2.f * ((b * c) + (a * d)), (2.f * (a2 + c2)) - 1.f, 2.f * ((c * d) - (a * b)), 0,

		2.f * ((b * d) - (a * c)), 2.f * ((c * d) + (a * b)), (2.f * (a2 + d2)) - 1.f, 0,

		0, 0, 0, 1
	);

	//return rotMat;
	return glm::transpose(rotMat);
}

glm::mat4 MatrixFunctions::rotateX(float angle)
{
	float sinPheta = sin(angle);
	float cosPheta = cos(angle);

	glm::mat4 rot = glm::mat4(
		1, 0, 0, 0,
		0, cosPheta, sinPheta, 0,
		0, -sinPheta, cosPheta, 0,
		0, 0, 0, 1
	);
	return glm::transpose(rot);
}

glm::mat4 MatrixFunctions::rotateY(float angle)
{
	float sinPheta = sin(angle);
	float cosPheta = cos(angle);

	glm::mat4 rot = glm::mat4(
		cosPheta, 0, sinPheta, 0,
		0, 1, 0, 0,
		-sinPheta, 0, cosPheta, 0,
		0, 0, 0, 1
	);
	return glm::transpose(rot);
}

glm::mat4 MatrixFunctions::rotateZ(float angle)
{
	float sinPheta = sin(angle);
	float cosPheta = cos(angle);

	glm::mat4 rot = glm::mat4(
		cosPheta, sinPheta, 0, 0,
		-sinPheta, cosPheta, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	return glm::transpose(rot);
}
