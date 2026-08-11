#include "SpaceJamMain.h"
#include "DrawObjects/DrawObject.h"
#include "SceneManager.h"
#include "AudioManager.h"
#include "DrawObjects/PlayerController.h"
#include "GUIManager.h"
#include "GUIObjects/GUIButton.h"
#include "GameManager.h"
#include "Renderer.h"
#include "OptionsManager.h"

#include "Utilities/ShaderLoader.h"
#include "Utilities/Framebuffer.h"
#include "Utilities/ObjectLoader.h"
#include <cmath>
#include <optional>
#include <stdexcept>
#include <glm/ext/vector_float3.hpp>
//Use radians instead of degrees
#define GLM_FORCE_RADIANS

const int screenHeight = 480;
const int screenWidth = 854;

glm::mat4 projection, modelview; 


//Remove this later
std::optional<GameManager> gameManager;
std::optional<Renderer> renderer;

std::map<unsigned char, bool> keyMap;

// ---------------------------------------------------------- GLUT FUNCTIONS ------------------------------------------------------------------

//This function displays a new frame
//This calls the GUIManager and ObjectManager render queues, it also causes a Game update
void display() {
	renderer->render(gameManager.value());
}

//The function that is called when the user changes the size of the window in which the game is in
//TODO: Reshaping doesn't even work right now, so don't even let a user do it
void reshape(int x, int y) {
	glViewport(0, 0, (GLsizei)x, (GLsizei)y);
	//screenHeight = y;
	//screenWidth = x;
	projection = glm::perspective(gameManager->fovy, (GLfloat)screenWidth/ (GLfloat)screenHeight, 1.0f, 200.0f);
	Program& shaderProgram = renderer->getShaderProgram();
	shaderProgram.setMat4("projection", projection);
}
	

//Called when the user clicks down on the mouse
void mouse([[maybe_unused]] int button, int state, int x, int y) {
	if (state == 1) {
		gameManager->setMouseClicked({x, screenHeight - y});
	}
}

//Called when the user moves the mouse
void mouseMotion(int x, int y) {
	gameManager->setMousePosition({x, screenHeight - y});
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
	
	//Describes how OpenGL should operate when drawing objects
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	//Necessary for rendering GUI, allows images to have a transparent background, this setting also allows a smoothing beteween them
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	keyMap.emplace('a', false);
	keyMap.emplace('d', false);
	renderer.emplace();
	gameManager.emplace(renderer->getShaderProgram(), renderer->getGUIProgram(), keyMap);

	//Glut manages most user input, these commands tell glut what functions to call on an input
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouseMotion);
	//Tells the program to call a new frame every 25ms
	glutTimerFunc(static_cast<int>(1000.f/60.f), newFrame, 0);
	glutKeyboardFunc(keyPress);
	glutKeyboardUpFunc(keyUp);

	//This command tells glut to start calling the newFrame function
	gameManager->Init();
	glutMainLoop();
	return 0;
}
