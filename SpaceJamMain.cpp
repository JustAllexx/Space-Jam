#include "SpaceJamMain.h"
//Use radians instead of degrees
#define GLM_FORCE_RADIANS

using namespace std;

int screenHeight = 480;
int screenWidth = 854;

//A constant global variable that defines the mathematical constant pi
//3.1415....
const double rotpi = 2 * acos(0.0);
const int kernelSize = 5;

//Time keeping variables
//dt is used by the audioManager to gauge how much time has passed since audio capture started
int newt, oldt;
float dt;

//This is where the integer locations of all the programIDs
//Once the program has been created OpenGL gives us a unique (unsigned) integer which we can use in an API call to tell OpenGL we want to use this shader in our rendering pipeline
//Scroll down to the CreatePrograms function for an explanation of each shader and it's purpose
GLuint vertexShader, fragmentShader, shaderProgram;
GLuint textVertexShader, textFragmentShader, textShaderProgram;
GLuint screenVertex, screenFragment, screenProgram;
GLuint debugVertex, debugFragment, debugProgram;
GLuint gaussianVertex, gaussianFragment, gaussianProgram;
GLuint projectionPos, modelviewPos;

//The projection and modelview are matrices which are defined for use in the vertex shader
//The modelview describes how the local space vertices should be converted into world space (translation, rotation and scaling)
//Projection describes how 3D vertices are converted into 2D screen coordinates
glm::mat4 projection, modelview; 
std::vector <glm::mat4> modelviewStack;

//Similar to the integers this is where the framebuffer IDs are stored. OpenGL handles these in a similar way
//Scroll down to the CreateFramebuffers function for an explanation of each framebuffer and its purpose
GLuint renderFramebuffer;
GLuint finalFramebuffer[2];
GLuint gaussianLeftBuffer[2];
GLuint gaussianRightBuffer[2];
GLuint debugFramebuffer[2];
GLuint splitColourBuffers[2];

//OpenGL ID for changing values in the fragment shader
//Here is where the location for the horizontal boolean value is stored in the gaussian shader program
//This ID can be used to change the value at this location from the main code
GLuint gaussianHorizontalPos;

//OpenGL ID for the render buffer object, the vertex array object, and the vertex buffer object
//Each one is necessary for rendering framebuffers to the screen
GLuint RBO;
GLuint screenVAO, screenVBO;

//This is the audioManager instance that is responsible for recording audio to the capture buffer
AudioManager audioManager;
PlayerController player;

std::map<char, bool> keyMap;

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

//A boolean variable that controls whether or not the RenderQueue function for the GUIManager is called
bool bRenderGui = true;

//Gaussian Functions
//This function calculates the gaussian distribution for the gaussian blur fragment shader
float gaussianDistribution(float x, float standardDeviation) {
	//Because we're normalising the weights in the kernel so they sum to 1, the usual constant the result needs to be multiplied by is not necessary here
	return exp((-0.5f * x * x) / (standardDeviation * standardDeviation));
}

//This function updates the gaussian blur kernel inside the gaussian blur fragment shader
//Higher values of standard deviation give a greater degree of blur while lower values give a much sharper blur
//The program is needed as an input to update the values at the end of the function
void updateGaussianKernel(float standardDeviation, GLuint program) {
	//The program needs to be loaded by OpenGL to update the values
	glUseProgram(program);

	//Where the gaussian values are stored
	std::valarray<float> kernelValues;
	kernelValues.resize(kernelSize);
	kernelValues[0] = 1.f;
	float sumValue = 1.f;
	//Because (when x = 0) is always 1 for out version of the distribution we can avoid this calculation
	for (int x = 1; x < kernelSize; x++) {
		float gaussianValue = gaussianDistribution(x, standardDeviation);
		kernelValues[x] = gaussianValue;
		//Multiply by two here because the gaussian kernel is symmetrical
		sumValue += 2 * gaussianValue;
	}

	//Normalise the kernel values so the sum of all the values (when expanded) is 1
	kernelValues /= sumValue;

	//Iterate through the kernel updating each value in the gaussian fragment shader's weight array
	//The weight array are what are used by the shader to calculate the gaussian blur
	for (int j = 0; j < kernelSize; j++) {
		//We need to access each weight value individually. So we create the weight location for the weight value we want
		//So weight[0] is the first value of our kernel
		std::string weightLocation = "weight[" + std::to_string(j) + "]";
		GLuint kernelWeightIndexPosition = glGetUniformLocation(program, weightLocation.c_str());
		//Update the value with the calculated value
		glUniform1f(kernelWeightIndexPosition, kernelValues[j]);
	}
}

// This function pushes the specified matrix onto the modelview stack
void pushMatrix(glm::mat4 mat) {
	modelviewStack.push_back(glm::mat4(mat));
}

// This function pops a matrix from the modelview stack and assigns that to the matrix passed in
void popMatrix(glm::mat4& mat) {
	if (modelviewStack.size()) {
		mat = glm::mat4(modelviewStack.back());
		modelviewStack.pop_back();
	}
	else { // Just to prevent errors when popping from an empty stack.
		mat = glm::mat4(1.0f);
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
	
	//This framebuffer was used for debugging purposes, if I wanted to render a framebuffer early, I would bind this framebuffer.
	//It is no longer used or needed in the code
	glGenFramebuffers(1, &debugFramebuffer[0]);
	glGenTextures(1, &debugFramebuffer[1]);
	glBindFramebuffer(GL_FRAMEBUFFER, debugFramebuffer[0]);
	glBindTexture(GL_TEXTURE_2D, debugFramebuffer[1]);
	framebufferSettings();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, debugFramebuffer[1], 0);

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

// -------------------------------------------------------------	REWRITE THIS CODE -----------------------------------------------------
//This is the code that loads shaders and compiles them using OpenGL calls.
GLuint loadShader(GLenum shaderType, string filename) {
	GLuint shader = glCreateShader(shaderType);
	string shaderText = readShaderFile(filename);

	const GLchar* c_str = shaderText.c_str();
	glShaderSource(shader, 1, &c_str, NULL);
	glCompileShader(shader);

	GLint bCompiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &bCompiled);
	if (!bCompiled) {
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* log = new GLchar[logLength + 1];
		glGetShaderInfoLog(shader, logLength, &logLength, log);
		cout << "Shader Compile Error\n" << log << endl;
	}

	return shader;
}

//This function creates programs
//programs are the name for a combination of a vertex and a fragment shader, this tells OpenGL to create a program that I can use later on when rendering and to link them to each other
GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
	//Creates a program and attaches the compiled vertex and fragment shader to it
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	GLint bLinked;
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &bLinked);

	if (bLinked) {
		glUseProgram(program);
	}
	else {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* log = new GLchar[logLength + 1];
		glGetProgramInfoLog(program, logLength, &logLength, log);
		cout << "Shader Compile Error\n" << log << endl;
	}

	return program;
}

//This function loads in a shader file and outputs the entire file as a string
//This is so it can be compiled later on in the code
//GLSL code is compiled during runtime unlike C++ code which is precompiled
string readShaderFile(string filename) {
	string ret;
	fstream shaderFile(filename);
	string nextLine;
	while (getline(shaderFile, nextLine)) {
		ret += nextLine + "\n";
	}
	return ret;
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
	glEnable(GL_DEPTH_TEST);
	//Load in the phong lighting shader
	glUseProgram(shaderProgram);

	GLuint lightPosPos = glGetUniformLocation(shaderProgram, "lightPos");
	glUniform3f(lightPosPos, player.posX, player.posY+2.f, 0.f);

	GameManager::gameUpdate();
	ObjectManager::renderQueue();

	if (bRenderGui) {
		glUseProgram(textShaderProgram);
		glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
		glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "textprojection"), 1, GL_FALSE, &textProjection[0][0]);
		GUIManager::renderQueue();
		glUseProgram(shaderProgram);
	}
	//I don't want the depth test to be enabled for rendering framebuffers, causes the framebuffer to not be seen
	glDisable(GL_DEPTH_TEST);
	
	//Gaussian blur
	//The guassian blur fragment shader is called repeatedly to blur the image drawn to Colour Attachment 1, switching between blurring horizontally and vertically
	glUseProgram(gaussianProgram);
	int ammount = 50;
	bool firstIteration = true;
	bool horizontalPass = true;
	for (int i = 0; i < ammount; i++) {
		glUniform1i(gaussianHorizontalPos, horizontalPass);

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
	glUseProgram(screenProgram);
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
void reshape(int x, int y) {
	glViewport(0, 0, (GLsizei)x, (GLsizei)y);
	screenHeight = y;
	screenWidth = x;
	projection = glm::perspective(GameManager::fovy, (GLfloat)x/ (GLfloat)y, 1.0f, 200.0f);
	glUniformMatrix4fv(projectionPos, 1, GL_FALSE, &projection[0][0]);
}

//Called when the user clicks down on the mouse
void mouse(int button, int state, int x, int y) {
	if (state == 1) {
		GUIManager::checkCollisions(x, screenHeight - y, 1);
	}
}

//Called when the user moves the mouse
void mouseMotion(int x, int y) {
	//std::cout << x << " " << y << std::endl;
	GUIManager::checkCollisions(x, screenHeight - y, 0);
}

//This is the function that is called to indicate a new frame should be rendered
void newFrame(int value) {
	float deltaSpeed;
	
	//Calculates the time since the last frame in seconds and stores the value in a float
	newt = glutGet(GLUT_ELAPSED_TIME);
	dt = (newt - oldt) / 1000.f;
	oldt = newt;
	
	//Asks the audio manager if there is a new frequency to be calculated
	double note, volume;
	note = 0.f;
	volume = 0.f;
	//If the audio manager returns 0, that means that no new frequency can be calculated because the capture buffer isn't filled yet
	//Or that the frequency calculated did not dip below the harmony threshold, so couldn't return an accurate value
	//This function also returns a volume, if the average volume (or gain) of the capture buffer was not above 400.f, then we ignore the value because the capture taken was too quiet
	audioManager.updateFrequency(dt, note, volume);
	if (note != 0 && volume > 400) {
		//Equation for calculating the piano key value of a frequency
		double key = (12 * log2(note / 440.f) + 49);
		//Can use this to determine the note was being sung
		key = std::fmod(key, 12);
		//this is passed on to a static function that calculates the height that the player should be on screen based on the value of the note sung
		float targetY = AudioManager::getHeightOfNote(key, GameManager::fovy, GameManager::dist);
		player.targetY = targetY;
	}
	//Update the players movement
	//Keymap contains what keys are being pressed down during this frame, dt is the time since last frame
	player.controlUpdate(keyMap, dt);
	//Call the display function
	glutPostRedisplay();
	glutTimerFunc(1000.0f / 60.0f, newFrame, value); // waits 16 ms before calling this function again
}

//Changes the keyMap to true or false depending on if a key has been pressed down or released
void keyPress(unsigned char key, int x, int y) {
	keyMap[key] = true;
}

void keyUp(unsigned char key, int x, int y) {
	keyMap[key] = false;
}

//Function that is called to start the game, calls the startGame function of the GameManager
void startGame() {
	GUIManager::showGameGUI();
	GameManager::startGame("Counting Stars Audio\\notes30s.json", "Counting Stars Audio\\CS_30s.ogg", &player);
}

//Terminates the program (with a 0 to signify no errors occured), the function that is called when quit is pressed from the main menu
void QuitGame() {
	exit(0);
}

//Function to load a compiled program, used in the graphics pipeline for rendering to the screen
GLuint loadProgram(const char* vertexShaderLoc, const char* fragmentShaderLoc) {
	vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderLoc);
	fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderLoc);
	return createProgram(vertexShader, fragmentShader);
}

void createPrograms() {
	//Loads in the gaussian vertex and fragment shaders, this program creates the bloom effect by blurring certain objects on the screen
	//This gives them the appearance that they are glowing
	gaussianProgram = loadProgram("Shaders\\gaussianBlur.vert", "Shaders\\gaussianBlur.frag");
	glUseProgram(gaussianProgram);
	//Update the weights of the kernel inside the gaussian blur program, horizontal is a boolean value which dictates whether the function should blur horizontally or vertically
	updateGaussianKernel(3.f, gaussianProgram);
	gaussianHorizontalPos = glGetUniformLocation(gaussianProgram, "horizontal");

	//Loads the program that is responsible for displaying the final framebuffer to the user
	screenProgram = loadProgram("Shaders\\screenShader.vert", "Shaders\\screenShader.frag");

	//Because the screenShader combines the bloomed texture and the rendered texture, it needs access to both textures
	//Here I specify which colour attachment belongs to which texture
	glUseProgram(screenProgram);
	GLuint screenTexturePos = glGetUniformLocation(screenProgram, "screenTexture");
	GLuint bloomBlurPos = glGetUniformLocation(screenProgram, "bloomBlur");
	glUniform1i(screenTexturePos, 0);
	glUniform1i(bloomBlurPos, 1);

	//Program responsible for displaying text (and all GUI Elements)
	//Uses orthogonal projection instead of perspective projection (like the objects in the scene). (Orthogonal projection makes it so that no matter how far away an object is from the screen, it's the same size)
	textShaderProgram = loadProgram("Shaders\\GUIShader.vert", "Shaders\\GUIShader.frag");
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(500.0f), 0.0f, static_cast<float>(500.0f));

	glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "textprojection"), 1, GL_FALSE, &textProjection[0][0]);
	//shaderProgram = loadProgram("vert.vert", "frag.frag");
	shaderProgram = loadProgram("Shaders\\PhongLighting.vert", "Shaders\\PhongLighting.frag");

	// Get the positions of the uniform variables
	projectionPos = glGetUniformLocation(shaderProgram, "projection");
	modelviewPos = glGetUniformLocation(shaderProgram, "modelview");
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
		cout << "Error in creating window";
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
	player.Setup("Textures\\goldenPlane2.png", shaderProgram);
	ObjectManager::Init(shaderProgram);
	GUIManager::Setup(textShaderProgram);
	OptionsManager::Initialise();

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

	GUIManager::showMainMenu();

	//Adds new objects to the scene to be rendered
	DrawObject* background = new DrawObject("Models\\nightSkyObj.obj", "Textures\\nightsky.png", false, 1.f, 1.f, false, glm::vec3(0.f, 4.f, 0.0f), glm::vec3(4.f, 4.f, 4.f), glm::vec3(0.f, rotpi, 0.f), glm::vec3(1.f, 1.f, 1.f));
	DrawObject* MoonObj = new DrawObject("Models\\moon.obj", "Textures\\moon.png", false, 1.f, 1.f, true, glm::vec3(50.f, 50.f, -100.f), glm::vec3(30.f, 30.f, 30.f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.f, 1.f, 1.f));
	MoonObj->setRotationalVelocity(glm::vec3(0.01f, 0.1f, 0.0f));
	
	//Start capturing audio for pitch calculations
	audioManager.StartCapture();

	//This command tells glut to start calling the newFrame function
	glutMainLoop();
	return 0;
}

//Defines the samplesGUI out of the scope of the header file
//These members belong to the class OptionsManager, but need to be defined out of the scope of the class before they can be used, because they are static values
buttonGUI* OptionsManager::samplesGUI = nullptr;
int OptionsManager::samplesOptionIndex = 0;
std::vector<std::string> OptionsManager::samplesOptionsText = {
	"1024 Samples",
	"2048 Samples",
	"4096 Samples",
	"512 Samples"
};

//In this function we define the function pointers for each function
//So when the start button is clicked the start game function is called
void OptionsManager::Initialise()
{
	GUIManager::MainMenu_StartButtonClick->setClickFunction(startGame);
	GUIManager::MainMenu_OptionsButtonClick->setClickFunction(GUIManager::showOptionsMenu);
	GUIManager::MainMenu_QuitButtonClick->setClickFunction(QuitGame);

	//Here we define what each button should do, what function it should call
	GUIManager::samplesOptionLeftClick->setClickFunction(DecrementSamplesOption);
	GUIManager::samplesOptionRightClick->setClickFunction(IncrementSamplesOption);
	GUIManager::samplesBackClick->setClickFunction(GUIManager::showMainMenu);
	samplesGUI = GUIManager::samplesOptionText;

	//Here we assign the string pointer of the score button gui to the player score text
	//This is what increments when a note is hit
	player.playerScoreText = &(GUIManager::GameGUI_ScoreText->text);
}

void OptionsManager::IncrementSamplesOption()
{
	//In C++ the modulus operator doesn't have the desired effect with negative numbers (-1 % 4) = -1 not 3 (which is what we want)
	//So add the size of the text vector on aswell
	samplesOptionIndex = (samplesOptionIndex + 1 + samplesOptionsText.size()) % samplesOptionsText.size();
	samplesGUI->text = samplesOptionsText[samplesOptionIndex];
}

void OptionsManager::DecrementSamplesOption()
{
	samplesOptionIndex = (samplesOptionIndex - 1 + samplesOptionsText.size()) % samplesOptionsText.size();
	samplesGUI->text = samplesOptionsText[samplesOptionIndex];
}
