#include "SpaceJamMain.h"
#include "DrawObjects/DrawObject.h"
#include "SceneManager.h"
#include "AudioManager.h"
#include "DrawObjects/PlayerController.h"
#include "GUIManager.h"
#include "GUIObjects/GUIButton.h"
#include "GameManager.h"
#include "OptionsManager.h"

#include <Utilities/ShaderLoader.h>
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <valarray>
#include <glm/ext/vector_float3.hpp>
//Use radians instead of degrees
#define GLM_FORCE_RADIANS

const int screenHeight = 480;
const int screenWidth = 854;
//Gaussian kernel
const size_t kernelSize = 5;

//Shader Paths
const char* phongVert = "Shaders/PhongLighting.vert";
const char* phongFrag = "Shaders/PhongLighting.frag";
const char* GUIVert = "Shaders/GUIShader.vert";
const char* GUIFrag = "Shaders/GUIShader.frag";
const char* screenVert = "Shaders/screenShader.vert";
const char* screenFrag = "Shaders/screenShader.frag";
const char* gaussianVert = "Shaders/gaussianBlur.vert";
const char* gaussianFrag = "Shaders/gaussianBlur.frag";

//This is where the integer locations of all the programIDs
//Once the program has been created OpenGL gives us a unique (unsigned) integer which we can use in an API call to tell OpenGL we want to use this shader in our rendering pipeline
//Scroll down to the CreatePrograms function for an explanation of each shader and it's purpose
std::optional<Program> shaderProgram;
std::optional<Program> guiProgram;
std::optional<Program> screenProgram;
std::optional<Program> gaussianProgram;
GLuint projectionPos, modelviewPos;

//The projection and modelview are matrices which are defined for use in the vertex shader
//The modelview describes how the local space vertices should be converted into world space (translation, rotation and scaling)
//Projection describes how 3D vertices are converted into 2D screen coordinates
glm::mat4 projection, modelview; 

//Similar to the integers this is where the framebuffer IDs are stored. OpenGL handles these in a similar way
//Scroll down to the CreateFramebuffers function for an explanation of each framebuffer and its purpose
GLuint renderFramebuffer;
GLuint finalFramebuffer[2];
GLuint gaussianLeftBuffer[2];
GLuint gaussianRightBuffer[2];
GLuint splitColourBuffers[2];

//OpenGL ID for the render buffer object, the vertex array object, and the vertex buffer object
//Each one is necessary for rendering framebuffers to the screen
GLuint RBO;
GLuint screenVAO, screenVBO;

//Remove this later
//std::optional<PlayerController> player;
std::optional<GameManager> gameManager;
std::optional<GUIManager> guiManager;
std::optional<OptionsManager> optionsManager;
//std::unique_ptr<SceneManager> sceneManager;

std::map<unsigned char, bool> keyMap;

//The vertex and UV coordinates of a quad
//This used when a framebuffer is rendered to the screen, the quad fills up the screen corner to corner
//Then the framebuffer texture is textured onto the drawn quad
float vertices[6][4] = {
	{-1.f, 1.f, 0.f, 1.f},
	{-1.f, -1.f, 0.f, 0.f},
	{1.f, -1.f, 1.f, 0.f},

	{-1.f, 1.f, 0.f, 1.f},
	{1.f, -1.f, 1.f, 0.f},
	{1.f, 1.f, 1.f, 1.f}
};

//Gaussian Functions
//This function calculates the gaussian distribution for the gaussian blur fragment shader
constexpr float gaussianDistribution(float x, float standardDeviation) {
	//Because we're normalising the weights in the kernel so they sum to 1, the usual constant the result needs to be multiplied by is not necessary here
	return expf((-0.5f * x * x) / (standardDeviation * standardDeviation));
}

//This function updates the gaussian blur kernel inside the gaussian blur fragment shader
//Higher values of standard deviation give a greater degree of blur while lower values give a much sharper blur
//The program is needed as an input to update the values at the end of the function
void updateGaussianKernel(float standardDeviation, Program& bloomProgram) {
	//The program needs to be loaded by OpenGL to update the values
	bloomProgram.use();

	//Where the gaussian values are stored
	std::valarray<float> kernelValues;
	kernelValues.resize(kernelSize);
	kernelValues[0] = 1.f;
	float sumValue = 1.f;
	//Because (when x = 0) is always 1 for out version of the distribution we can avoid this calculation
	for (size_t x = 1; x < kernelSize; x++) {
		float gaussianValue = gaussianDistribution(static_cast<float>(x), standardDeviation);
		kernelValues[x] = gaussianValue;
		//Multiply by two here because the gaussian kernel is symmetrical
		sumValue += 2 * gaussianValue;
	}

	//Normalise the kernel values so the sum of all the values (when expanded) is 1
	kernelValues /= sumValue;

	//Iterate through the kernel updating each value in the gaussian fragment shader's weight array
	//The weight array are what are used by the shader to calculate the gaussian blur
	for (size_t j = 0; j < kernelSize; j++) {
		//We need to access each weight value individually. So we create the weight location for the weight value we want
		//So weight[0] is the first value of our kernel
		std::string weightLocation = "weight[" + std::to_string(j) + "]";
		//GLuint kernelWeightIndexPosition = glGetUniformLocation(program, weightLocation.c_str());
		//Update the value with the calculated value
		//glUniform1f(kernelWeightIndexPosition, kernelValues[j]);
		bloomProgram.setFloat(weightLocation, kernelValues[j]);
	}
}

//This function is used to render a quad from corner to corner of the screen
//This is useful because we can bind a texture before we call this function and that texture will be rendered across the whole screen
//This can be used to render the contents of framebuffers, this is incredibly useful if I want to pass multiple fragment shaders over one image
//Displaying a framebuffer allows me to run a shader over it again, and as often as I like until I finally display it to the user
void displayFramebuffer() {
	//Unbinds any previously bound vertex array object
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0); //Activates the first texture slot
	//Binds the vertex array and buffer into OpenGL for rendering, we're telling OpenGL we want to render a quad
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	//Buffer in the vertex data of the quad I want to render to the screen.
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	//Finally draw it, there are 6 vertices required to draw a quad, 3 for each face (since we're drawing with exclusively triangles).
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//I create a handful of framebuffers in the createFramebuffers function, this needs to be specified for every single framebuffer
//As long as I bind the framebuffer beforehand, I can call this code to setup the framebuffer correctly and improve readability
void framebufferSettings() {
	//Creates a texture for the framebuffer that is the same height and width as the screen and stores all RGB and alpha value channels in a 16 bit float (each)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	//If the framebuffer should ever be rendered at a smaller size, the GPU should use linear interpolation to scale it up or down
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//If the fragment shader ever asks for a pixel that is out of bounds of the texture, it will just wrap around to the other end of the image
	//S and T are the directions 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//This function is responsible for creating every framebuffer my programme is going to need
void createFramebuffers() {
	
	//The finalframebuffer is where the final image is rendered to, this is the image the end user sees
	glGenFramebuffers(1, &finalFramebuffer[0]);
	glGenTextures(1, &finalFramebuffer[1]);
	glBindFramebuffer(GL_FRAMEBUFFER, finalFramebuffer[0]);
	glBindTexture(GL_TEXTURE_2D, finalFramebuffer[1]);
	framebufferSettings();
	//Adds a texture object to the framebuffer as a colour attachment, whenever an object is rendered it will be drawn to this texture (if the framebuffer is binded)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalFramebuffer[1], 0);

	//The gaussian blur requires two framebuffers that are switched between a handful of times before rendering
	//The left buffer is concerned with horizontal blurring, the right buffer is for vertical blurring
	glGenFramebuffers(1, &gaussianLeftBuffer[0]);
	glGenTextures(1,  &gaussianLeftBuffer[1]);
	glBindFramebuffer(GL_FRAMEBUFFER, gaussianLeftBuffer[0]);
	glBindTexture(GL_TEXTURE_2D, gaussianLeftBuffer[1]);
	framebufferSettings();

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gaussianLeftBuffer[1], 0);
	glGenFramebuffers(1, &gaussianRightBuffer[0]);
	glGenTextures(1, &gaussianRightBuffer[1]);
	glBindFramebuffer(GL_FRAMEBUFFER, gaussianRightBuffer[0]);
	glBindTexture(GL_TEXTURE_2D, gaussianRightBuffer[1]);
	framebufferSettings();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gaussianRightBuffer[1], 0);
	

	//This is the framebuffer where everything is initially rendered to
	//The ObjectManager renders to this framebuffer, this framebuffer is not displayed to the user.
	glGenFramebuffers(1, &renderFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
	glGenTextures(2, splitColourBuffers);
	for (unsigned int i = 0; i < 2; i++) {
		//This section is needed for the gaussian blur
		//I attach to colour attachments to the renderBuffer, every object is rendered to Colour Attachment 0, but objects that I want blurred get rendered to Colour Attachment 1
		glBindTexture(GL_TEXTURE_2D, splitColourBuffers[i]);
		framebufferSettings();

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, splitColourBuffers[i], 0);
	}
	//This creates the renderbuffer needed to display each framebuffer
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
	
	//Tells OpenGL I'm interested in rendering to two textures (or colour attachments)
	//The colour attachments are how I control what gets rendered to each texture
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
}

// ---------------------------------------------------------- GLUT FUNCTIONS ------------------------------------------------------------------

//This function displays a new frame
//This calls the GUIManager and ObjectManager render queues, it also causes a Game update
void display() {
	//Binds the framebuffer that I want the ObjectManager to render every object to
	glBindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
	//Tells OpenGL to clear the screen completely and replace it with black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear all information about what colour the image is and clear information about which pixel of the previous frame was closest to the camera
	//If Object 1 is behind Object 2, but Object 1 is rendered after Object 2. Object 1 will appear on top of Object 2
	//This feature is provided by OpenGL so that if a pixel is supposed to be behind another object. OpenGL will ignore it
	//TODO: This can always stay on, so put this in some sort of setup
	glEnable(GL_DEPTH_TEST);
	//Load in the phong lighting shader

	gameManager->render();
	
	//Gaussian blur
	//The guassian blur fragment shader is called repeatedly to blur the image drawn to Colour Attachment 1, switching between blurring horizontally and vertically
	//glUseProgram(gaussianProgram->getProgramID());
	gaussianProgram->use();
	int ammount = 50;
	bool firstIteration = true;
	bool horizontalPass = true;
	for (int i = 0; i < ammount; i++) {
		gaussianProgram->setInt("horizontal", horizontalPass);

		if (horizontalPass) {
			glBindFramebuffer(GL_FRAMEBUFFER, gaussianLeftBuffer[0]);
			if (firstIteration) {
				glBindTexture(GL_TEXTURE_2D, splitColourBuffers[1]);
				firstIteration = false;
			}
			else {
				glBindTexture(GL_TEXTURE_2D, gaussianRightBuffer[1]);
			}
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, gaussianRightBuffer[0]);
			glBindTexture(GL_TEXTURE_2D, gaussianLeftBuffer[1]);
		}
		displayFramebuffer();
		horizontalPass = !horizontalPass;
	}
	
	//This is the final render to the screen
	//The screen program (vertex shader and fragment shader) combines the Colour Attachment 0 texture with the blurred Colour Attachment 1 texture
	//This gives the completed bloom effect
	//glUseProgram(screenProgram->getProgramID());
	screenProgram->use();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, splitColourBuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gaussianRightBuffer[1]);
	displayFramebuffer();
	//Clears the vertex buffer and clears the texture buffer
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	//Swaps the display buffer so the user is finally presented with the image
	glutSwapBuffers();
	//Clear any rendering commands in the GPU that may still be being processed
	glFlush();
}

//The function that is called when the user changes the size of the window in which the game is in
//TODO: Reshaping doesn't even work right now, so don't even let a user do it
void reshape(int x, int y) {
	glViewport(0, 0, (GLsizei)x, (GLsizei)y);
	//screenHeight = y;
	//screenWidth = x;
	projection = glm::perspective(gameManager->fovy, (GLfloat)screenWidth/ (GLfloat)screenHeight, 1.0f, 200.0f);
	glUniformMatrix4fv(projectionPos, 1, GL_FALSE, &projection[0][0]);
}
	

//Called when the user clicks down on the mouse
void mouse([[maybe_unused]] int button, int state, int x, int y) {
	if (state == 1) {
		guiManager->checkCollisions(x, screenHeight - y, 1);
	}
}

//Called when the user moves the mouse
void mouseMotion(int x, int y) {
	//std::cout << x << " " << y << std::endl;
	guiManager->checkCollisions(x, screenHeight - y, 0);
}

//This is the function that is called to indicate a new frame should be rendered
void newFrame(int value) {	
	gameManager->gameUpdate();

	//Call the display function
	glutPostRedisplay();
	unsigned int nextFrameTime = static_cast<unsigned int>(1000.f / 60.f);
	glutTimerFunc(nextFrameTime, newFrame, value); // waits 16 ms before calling this function again
}

//Changes the keyMap to true or false depending on if a key has been pressed down or released
void keyPress(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y) {
	keyMap[key] = true;
}

void keyUp(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y) {
	keyMap[key] = false;
}

//Initialise all the Shaders / Programs that Space Jam needs
void createPrograms() {
	//Loads in the gaussian vertex and fragment shaders, this program creates the bloom effect by blurring certain objects on the screen
	//Update the weights of the kernel inside the gaussian blur program
	gaussianProgram.emplace(gaussianVert, gaussianFrag);
	updateGaussianKernel(3.f, gaussianProgram.value());

	//Loads the program that is responsible for displaying the final framebuffer to the user
	screenProgram.emplace(screenVert, screenFrag);
	//Because the screenShader combines the bloomed texture and the rendered texture, it needs access to both textures
	//Here I specify which colour attachment belongs to which texture
	screenProgram->setInt("screenTexture", 0);
	screenProgram->setInt("bloomBlur", 1);

	//Program responsible for displaying text (and all GUI Elements)
	//Uses orthogonal projection instead of perspective projection (like the objects in the scene). (Orthogonal projection makes it so that no matter how far away an object is from the screen, it's the same size)
	//textShaderProgram = loadProgram("Shaders/GUIShader.vert", "Shaders/GUIShader.frag");
	guiProgram.emplace(GUIVert, GUIFrag);
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(500.0f), 0.0f, static_cast<float>(500.0f));
	//glUniformMatrix4fv(glGetUniformLocation(guiProgram->getProgramID(), "textprojection"), 1, GL_FALSE, &textProjection[0][0]);
	guiProgram->setMat4("textprojection", textProjection);

	//Shader program initialisation
	shaderProgram.emplace(phongVert, phongFrag);
	// Get the positions of the uniform variables
	projectionPos = glGetUniformLocation(shaderProgram->getProgramID(), "projection");
	modelviewPos = glGetUniformLocation(shaderProgram->getProgramID(), "modelview");
	// Pass the projection and modelview matrices to the shader
	glUniformMatrix4fv(projectionPos, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(modelviewPos, 1, GL_FALSE, &(modelview)[0][0]);
}

//This is the function that is called when the program is executed
//argc and argv are optional arguments that can be passed through if the program is executed from the command line
//Because I don't expect this to happen I assume both these arguments will be empty

//The main function is in charge of instantiating glut and creating the window and calling the initialisation of the other classes
int main(int argc, char** argv) {
	//Inititates glut and allows us to create the window
	glutInit(&argc, argv);
	//We tell glut that we want to use a double buffer, this means we can draw to one buffer while another buffer is being drawn
	//We also tell glut that we want our display to have a Red, Green, Blue, and alpha channel
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(screenWidth, screenHeight);
	glutCreateWindow("Space Jam");
	//Create the window and prints to the console if anything went wrong
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		throw std::runtime_error("Unable to create GLEW window");
	}
	//When we clear the screen what do we write over the buffer with, tells OpenGL I want an empty buffer to completely black and transparent
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	//Creates a quad that can be rendered onto the screen
	//Useful for displaying framebuffers
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	//Describes how OpenGL should operate when drawing objects
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	//Necessary for rendering GUI, allows images to have a transparent background, this setting also allows a smoothing beteween them
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	//Creates the Framebuffers and the shader programs
	createFramebuffers();
	createPrograms();

	//Class initialisation functions
	//objectManager.emplace(shaderProgram);

	keyMap.emplace('a', false);
	keyMap.emplace('d', false);
	guiManager.emplace(guiProgram.value());
	gameManager.emplace(shaderProgram.value(), keyMap, &guiManager.value());
	optionsManager.emplace(guiManager.value(), &gameManager.value());

	//Glut manages most user input, these commands tell glut what functions to call on an input
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouseMotion);
	//Tells the program to call a new frame every 25ms
	glutTimerFunc(100/20, newFrame, 0);
	glutKeyboardFunc(keyPress);
	glutKeyboardUpFunc(keyUp);

	//This command tells glut to start calling the newFrame function
	gameManager->Init();
	glutMainLoop();
	return 0;
}
