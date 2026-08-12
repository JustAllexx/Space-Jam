#include "Application.h"
#include <stdexcept>

const int screenHeight = 480;
const int screenWidth = 854;

Application* Application::inst{nullptr};
std::unordered_map<unsigned char, bool> Application::keyMap;

Application::Application(int argc, char** argv)
{
    inst = this;
    keyMap.emplace('a', false);
    keyMap.emplace('d', false);

    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(screenWidth, screenHeight);
	glutCreateWindow("Space Jam");
    if (glewInit() != GLEW_OK) {throw std::runtime_error("Unable to create GLEW window");}
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	//Necessary for rendering GUI, allows images to have a transparent background, this setting also allows a smoothing beteween them
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

    //Glut manages most user input, these commands tell glut what functions to call on an input
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutDisplayFunc(Application::displayCallback);
	glutReshapeFunc(Application::reshapeCallback);
	glutMouseFunc(Application::mouseCallback);
	glutPassiveMotionFunc(Application::mouseMotionCallback);
	//Tells the program to call a new frame every 25ms
	glutTimerFunc(static_cast<int>(1000.f/60.f), Application::newFrameCallback, 0);
	glutKeyboardFunc(Application::keyPressCallback);
	glutKeyboardUpFunc(Application::keyUpCallback);

    renderer = std::make_unique<Renderer>();
    gameManager = std::make_unique<GameManager>(renderer->getShaderProgram(), renderer->getGUIProgram(), keyMap);

    gameManager->Init();
}

void Application::Run() {
    glutMainLoop();
}

void Application::display() {
    renderer->render(*gameManager);
}

void Application::newFrame(int value) {
    gameManager->gameUpdate();

	//Call the display function
	glutPostRedisplay();
	unsigned int nextFrameTime = static_cast<unsigned int>(1000.f / 60.f);
	glutTimerFunc(nextFrameTime, Application::newFrameCallback, value); // waits 16 ms before calling this function again
}

void Application::reshape(int x, int y) {
    glViewport(0, 0, x, y);
	auto aspect = static_cast<GLfloat>(screenWidth) / static_cast<GLfloat>(screenHeight);
	glm::mat4 projection = glm::perspective(gameManager->fovy, aspect, 1.0f, 200.0f);
	Program& shaderProgram = renderer->getShaderProgram();
	shaderProgram.setMat4("projection", projection);
}

void Application::mouse([[maybe_unused]] int button, int state, int x, int y) {
	if (state == 1) {
		gameManager->setMouseClicked({x, screenHeight - y});
	}
}

void Application::mouseMotion(int x, int y) {
    gameManager->setMousePosition({x, screenHeight - y});
}

void Application::keyPress(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y) {
    keyMap[key] = true;
}

void Application::keyUp(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y) {
    keyMap[key] = false;
}