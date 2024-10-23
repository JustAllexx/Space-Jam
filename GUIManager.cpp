#include "GUIManager.h"

#include FT_FREETYPE_H

//Similar to how a framebuffer is rendered onto a quad, so are characters, every character in a string is rendered to a quad and then has a texture applied over it
GLuint VAO, VBO;

//A C++ Structure, stores details about every character that can be rendered to the screen
//Every character in a font will have these details
struct TypeChar {
	GLuint TextureID;
	glm::vec2 Size;
	glm::vec2 Bearing;
	unsigned int Advance;
};

//
std::map<char, TypeChar> fontMap;
std::vector<GUIObject*> guiRenderQueue;
std::vector<Clickable*> clickChecks;
//GUIShader is where the program id for the GUI Rendering shader is stored in OpenGL, isText is a uniform value telling the shader if it's rendering text or an image
GLuint GUIshader, isTextPos;

//Code that takes in a mouse location and outputs whether the click was within a clickable objects region
bool Clickable::checkCollision(int mousePosX, int mousePosY) {
	return bottom < mousePosY&& mousePosY < top&&
		left < mousePosX&& mousePosX < right;
}

//Setter functions for the function pointers of a clickable object, used for defining new button behaviour
void Clickable::setClickFunction(void(*newClickFunction)())
{
	onClick = newClickFunction;
}

void Clickable::setHoverFunction(void(*newHoverFunction)())
{
	onHover = newHoverFunction;
}

//Construction function for the button GUI Class
buttonGUI::buttonGUI(std::string inText, float inX, float inY, float inScale, GLfloat colR, GLfloat colG, GLfloat colB, GLfloat hovR, GLfloat hovG, GLfloat hovB, void (*f)()) {
	{
		//Sets input variables as class variables
		x = inX; y = inY; scale = inScale;
		text = inText;
		if (f != nullptr) { //Blank case, if function is equal to nullptr, don't assign it to onClick
			onClick = f;
		}

		colour[0] = colR; colour[1] = colG; colour[2] = colB;
		hoverColour[0] = hovR; hoverColour[1] = hovG; hoverColour[2] = hovB;

		//Advance Sum calculates how big the text is based on the "advance" and "scale" of each character loaded in by the font loader
		advanceSum = 0;
		maxHeight = 0;
		//Iterates through every character in a string
		std::string::const_iterator tempIt;
		for (tempIt = text.begin(); tempIt != text.end(); tempIt++) {
			TypeChar tempChar = fontMap[*tempIt];
			advanceSum += (tempChar.Advance >> 6) * scale;
			//Gets the maximum height by comparing the height of every character in the text
			maxHeight = std::max(maxHeight, tempChar.Size.y * scale);
		}
		advanceSum = advanceSum / 2;

		//Clickable Directions (Coordinates for where the click region starts and ends based on the text
		right = x + advanceSum;
		left = x - advanceSum;
		top = inY + std::ceil(maxHeight);
		bottom = inY;
		top -= (maxHeight) / 2;
		bottom -= (maxHeight / 2);
		
		//Enable the button
		enable = true;
	}
}

//Override for the ObjectGUI Render
//Renders each character in a string, renders each character indivudally 
void buttonGUI::Render() {
	//Tell the GUI Shader Programme that I am rendering text
	glProgramUniform1i(GUIshader, isTextPos, 1);
	//If the text is being hovered over, set it to the hover colour, if not to the deafult colour
	if (hovered) {
		glUniform3f(glGetUniformLocation(GUIshader, "textColor"), hoverColour[0], hoverColour[1], hoverColour[2]);
	}
	else {
		glUniform3f(glGetUniformLocation(GUIshader, "textColor"), colour[0], colour[1], colour[2]);
	}

	if (!enable) { //If the button is not enabled don't render it
		return;
	}
	//Binds the information for drawing quads
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	float charx = x;

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		//The character struct for each character being rendered
		TypeChar ch = fontMap[*c];

		float xpos = (charx - advanceSum) + ch.Bearing.x * scale;
		float ypos = (y - (maxHeight / 2)) - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// the Quad coordinates for each character being rendered
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		charx += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	//Clear the vertex array and the texture once finished rendering
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//Simple constructor function for the ImageGUI class, loads in an image using the stbi image loader then buffers it to a texture
imageGUI::imageGUI(const char* imagePath, float inX, float inY, float inScale) {
	posX = inX; posY = inY; scale = inScale;

	stbi_set_flip_vertically_on_load(false);
	int numChannels;
	unsigned char* bytes = stbi_load(imagePath, &imgWidth, &imgHeight, &numChannels, 0);

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(bytes);
}

//Render Function for ImagGUI class, an override for the GUIObject class, very similar to rendering a quad for a character, but calculates the coordinates of the quad slightly differently
void imageGUI::Render() {
	glDisable(GL_DEPTH_TEST);
	//Tell the GUI Program shader that I'm not rendering text and it should render an image
	glProgramUniform1i(GUIshader, isTextPos, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	float h = (imgHeight * scale) * 0.5f;
	float w = (imgWidth * scale) * 0.5f;

	float vertices[6][4] = {
			{ posX - w,     posY + h,   0.0f, 0.0f },
			{ posX - w,     posY - h,       0.0f, 1.0f },
			{ posX + w , posY - h,       1.0f, 1.0f },

			{ posX - w,     posY + h,   0.0f, 0.0f },
			{ posX + w, posY - h,       1.0f, 1.0f },
			{ posX + w, posY + h,   1.0f, 0.0f }
	};

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// render quad
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}

//The setup function for the GUI Manager
void GUIManager::Setup(GLuint program) {
	//Uses FT to load fonts
	FT_Library ft;
	GUIshader = program;

	if (FT_Init_FreeType(&ft)) { throw "Failed to open FT Library"; return; }

	FT_Face face;
	if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face)) { throw "Failed to open font"; return; }
	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	//Iterates over the unicode characters from 0-127, gives the basic ASCII character set
	for (unsigned int c = 0; c < 128; c++) {
		//Uses the FT library to load character for specific character
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) { std::cout << "Failed to load glyph" << std::endl; continue; }

		//Generate the texture for this specific character
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RED, //Characters don't have colour by default, so only need 1 Colour channel
			face->glyph->bitmap.width, //Width of the specific character (or glyph)
			face->glyph->bitmap.rows, //Height of the character
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer //byte content of the image of the character (rendered similarly to an image)
		);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		//Define the values for a structure 
		TypeChar character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		//Insert the struct into a map so that properties about the character being rendered can be requested on rendering
		fontMap.insert(std::pair<char, TypeChar>(c, character));
	}
	//Clears the buffer of the FT font loader library
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	//Buffers a quad (similar to what happens in the Main Module
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	guiRenderQueue = std::vector<GUIObject*>();
	//Requests the uniform location of the isText bool in the GUI Shader
	isTextPos = glGetUniformLocation(GUIshader, "isText");

	//Creates all the GUI scenes
	createMainMenu();
	createOptionsMenu();
	createGameGUI();
	createScoreMenu();
}

//Iterates over the guiRenderQueue and calls the render function of every GUI Element that is meant to be on screen
void GUIManager::renderQueue() {
	for (size_t i = 0; i < guiRenderQueue.size(); i++) {
		if (guiRenderQueue[i]) {
			guiRenderQueue[i]->Render();
		}
	}
}

//Called every time the mouse is moved or clicked
//Iterates through every clickable GUI element in the scene, calls the onClick function if the mouse was clicked, sets hover to true if it was only hovered over
void GUIManager::checkCollisions(int mousePosX, int mousePosY, bool clicked) {
	for (Clickable* check : clickChecks) {
		//std::cout << mousePosX << " " << mousePosY << std::endl;
		if (check->checkCollision(mousePosX, mousePosY)) {	
			//Check that the button has a behaviour when clicked, if the value is nullptr the button hasn't been assigned a function
			if (clicked && check->onClick != nullptr) {
				(*check->onClick)();
			}
			else {
				check->hovered = true;
			}
		}
		else {
			check->hovered = false;
		}
	}
}

//Avoids unresolved external symbol errors, anything defined in a header file must also be defined in the main scope, even if I intend to overwrite it later
//These are all default values

/*

	-The rest of the code describes scenes and what GUIElements should compose them
	-ButtonGUI's are defined here (out of scope) so that their behaviours can be defined in the main module
	-Every Create function creates a series of instances of GUIObjects and appends them to an array which is then copied onto the renderQueue when the scene is meant to be shown

*/
buttonGUI* GUIManager::samplesOptionLeftClick = nullptr;
buttonGUI* GUIManager::samplesOptionRightClick = nullptr;
buttonGUI* GUIManager::samplesOptionText = nullptr;
buttonGUI* GUIManager::samplesBackClick = nullptr;

std::vector<GUIObject*> GUIManager::optionsMenuVector = std::vector<GUIObject*>();
std::vector<Clickable*> GUIManager::optionsMenuClickables = std::vector<Clickable*>();

buttonGUI* GUIManager::MainMenu_StartButtonClick = nullptr;
buttonGUI* GUIManager::MainMenu_OptionsButtonClick = nullptr;
buttonGUI* GUIManager::MainMenu_QuitButtonClick = nullptr;

std::vector<GUIObject*> GUIManager::mainMenuVector = std::vector<GUIObject*>();
std::vector<Clickable*> GUIManager::mainMenuClickables = std::vector<Clickable*>();

buttonGUI* GUIManager::GameGUI_ScoreText = nullptr;

std::vector<GUIObject*> GUIManager::gameGUIVector = std::vector<GUIObject*>();
std::vector<Clickable*> GUIManager::gameGUIClickables = std::vector<Clickable*>();

buttonGUI* GUIManager::scoreScreen_FinalScoreText = nullptr;

std::vector<GUIObject*> GUIManager::scoreMenuVector = std::vector<GUIObject*>();
std::vector<Clickable*> GUIManager::scoreMenuClickables = std::vector<Clickable*>();

void GUIManager::createOptionsMenu()
{
	int screenHeight = 480;
	int screenWidth = 854;

	GUIObject* background = new imageGUI("Textures\\holder.png", screenWidth / 2, screenHeight / 2, 10);
	
	GUIObject* pitchAccuracyText = new buttonGUI("Audio Buffer Size", screenWidth / 2, 350.f, 1,
		0.7f, 0.7f, 0.7f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	
	buttonGUI* samplesOptionTextTemp = new buttonGUI("1024 Samples", screenWidth / 2, 300.f, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	GUIManager::samplesOptionText = samplesOptionTextTemp;
	buttonGUI* samplesOptionLeftClickTemp = new buttonGUI("<", (screenWidth / 2) - 200.f, 300.f, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	GUIManager::samplesOptionLeftClick = samplesOptionLeftClickTemp;

	buttonGUI* samplesOptionRightClickTemp = new buttonGUI(">", (screenWidth / 2) + 200.f, 300.f, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	GUIManager::samplesOptionRightClick = samplesOptionRightClickTemp;

	buttonGUI* backButton = new buttonGUI("Back", screenWidth / 2, 100.f, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	GUIManager::samplesBackClick = backButton;
	
	optionsMenuVector.push_back(pitchAccuracyText);
	optionsMenuVector.push_back(background);
	optionsMenuVector.push_back(backButton);
	optionsMenuVector.push_back(samplesOptionText);
	optionsMenuVector.push_back(samplesOptionLeftClick);
	optionsMenuVector.push_back(samplesOptionRightClick);

	optionsMenuClickables.push_back(samplesOptionLeftClick);
	optionsMenuClickables.push_back(samplesOptionRightClick);
	optionsMenuClickables.push_back(backButton);
	
}

void GUIManager::showOptionsMenu()
{
	guiRenderQueue.clear();
	guiRenderQueue.assign(optionsMenuVector.begin(), optionsMenuVector.end());
	clickChecks.clear();
	clickChecks.assign(optionsMenuClickables.begin(), optionsMenuClickables.end());
}

void GUIManager::createMainMenu()
{
	int screenHeight = 480;
	int screenWidth = 854;

	GUIObject* logoImage = new imageGUI("Textures\\logo.png", screenWidth / 2, screenHeight - 100.f, 1.4f);

	buttonGUI* startButton = new buttonGUI("Start", screenWidth / 2, 250, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	GUIManager::MainMenu_StartButtonClick = startButton;

	buttonGUI* optionsButton = new buttonGUI("Options", screenWidth / 2, 175, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	GUIManager::MainMenu_OptionsButtonClick = optionsButton;

	buttonGUI* quitButton = new buttonGUI("Quit", screenWidth / 2, 100, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	GUIManager::MainMenu_QuitButtonClick = quitButton;

	GUIObject* testImage = new imageGUI("Textures\\pop_cat.png", 175, 200, 1);

	mainMenuVector.push_back(logoImage);
	mainMenuVector.push_back(startButton);
	mainMenuVector.push_back(optionsButton);
	mainMenuVector.push_back(quitButton);
	
	mainMenuClickables.push_back(startButton);
	mainMenuClickables.push_back(optionsButton);
	mainMenuClickables.push_back(quitButton);
}

void GUIManager::showMainMenu()
{
	guiRenderQueue.clear();
	guiRenderQueue.assign(mainMenuVector.begin(), mainMenuVector.end());
	clickChecks.clear();
	clickChecks.assign(mainMenuClickables.begin(), mainMenuClickables.end());
}

void GUIManager::createGameGUI()
{
	int screenHeight = 480;
	int screenWidth = 854;

	std::vector<std::string> noteText = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };

	for (int i = 1; i < 13; i++) {
		float percentHeight = (i / 13.f);
		float textHeight = percentHeight * screenHeight;

		//std::cout << noteText[i - 1] << " " << percentHeight << std::endl;

		buttonGUI* tempText = new buttonGUI(noteText[i - 1], screenWidth - 50.f, textHeight, .4f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, nullptr);
		gameGUIVector.push_back(tempText);
	}

	buttonGUI* scoreGUI = new buttonGUI("0", screenWidth / 2, screenHeight - 100, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, nullptr);
	GUIManager::GameGUI_ScoreText = scoreGUI;
	gameGUIVector.push_back(scoreGUI);
}

void GUIManager::showGameGUI()
{
	guiRenderQueue.clear();
	guiRenderQueue.assign(gameGUIVector.begin(), gameGUIVector.end());
}

void GUIManager::createScoreMenu()
{
	int screenHeight = 480;
	int screenWidth = 854;

	GUIObject* background = new imageGUI("Textures\\holder.png", screenWidth / 2, screenHeight / 2, 10);

	buttonGUI* backButton = new buttonGUI("Back", screenWidth / 2, 100.f, 1,
		0.5f, 0.5f, 0.5f,
		0.7f, 0.7f, 0.7f,
		showMainMenu);

	buttonGUI* yourScoreText = new buttonGUI("Your Score:", screenWidth / 2.f, 340.f, 1.2f,
		0.7f, 0.7f, 0.7f,
		0.7f, 0.7f, 0.7f
		, nullptr);

	buttonGUI* scoreScreenText = new buttonGUI("0", screenWidth / 2, 250.f, 1,
		0.7f, 0.7f, 0.7f,
		0.7f, 0.7f, 0.7f,
		nullptr);
	scoreScreen_FinalScoreText = scoreScreenText;

	scoreMenuVector.push_back(background);
	scoreMenuVector.push_back(backButton);
	scoreMenuVector.push_back(yourScoreText);
	scoreMenuVector.push_back(scoreScreenText);

	scoreMenuClickables.push_back(backButton);
}

void GUIManager::showScoreMenu()
{
	guiRenderQueue.clear();
	guiRenderQueue.assign(scoreMenuVector.begin(), scoreMenuVector.end());
	clickChecks.clear();
	clickChecks.assign(scoreMenuClickables.begin(), scoreMenuClickables.end());
}

