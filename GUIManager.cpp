#include "GUIManager.h"
#include "GUIObjects/GUIImage.h"
#include "GUIObjects/GUIButton.h"
#include "SceneManager.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <iostream>

#include FT_FREETYPE_H

const glm::vec3 white(1.0f, 1.0f, 1.0f);
const glm::vec3 lightGray(0.7f, 0.7f, 0.7f);
const glm::vec3 darkGray(0.5f, 0.5f, 0.5f);

//TODO: Absolutely remove this when implementing dynamically sizing the screen
const float screenHeight = 480.f;
const float screenWidth = 854.f;

const char* arialPath = "fonts/arial.ttf";

GUIManager::GUIManager(GLuint program) : arialFont(std::make_unique<SJ_Font>(arialPath)),
  GUIShader(program) {
	//Buffers a quad (similar to what happens in the Main Module
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	//Creates all the GUI scenes
	createMainMenu();
	createOptionsMenu();
	createGameGUI();
	createScoreMenu();
  }

//Iterates over the guiRenderQueue and calls the render function of every GUI Element that is meant to be on screen
void GUIManager::renderQueue() {
	glUseProgram(GUIShader);
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
	glUniformMatrix4fv(glGetUniformLocation(GUIShader, "textprojection"), 1, GL_FALSE, &textProjection[0][0]);
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
				(check->onClick)();
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

void GUIManager::createOptionsMenu()
{
	GUIObject* background = new GUIImage("Textures\\holder.png", screenWidth / 2, screenHeight / 2, 10, VAO, VBO, GUIShader);
	
	GUIObject* pitchAccuracyText = new GUIButton("Audio Buffer Size", screenWidth / 2, 350.f, 1,
		lightGray,
		lightGray,
		nullptr,
		*arialFont, VAO, VBO, GUIShader);
	
	GUIButton* samplesOptionTextTemp = new GUIButton("1024 Samples", screenWidth / 2, 300.f, 1,
		darkGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
	GUIManager::samplesOptionText = samplesOptionTextTemp;
	GUIButton* samplesOptionLeftClickTemp = new GUIButton("<", (screenWidth / 2) - 200.f, 300.f, 1,
		darkGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
	GUIManager::samplesOptionLeftClick = samplesOptionLeftClickTemp;

	GUIButton* samplesOptionRightClickTemp = new GUIButton(">", (screenWidth / 2) + 200.f, 300.f, 1,
		darkGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
	GUIManager::samplesOptionRightClick = samplesOptionRightClickTemp;

	GUIButton* backButton = new GUIButton("Back", screenWidth / 2, 100.f, 1,
		darkGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
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
	GUIObject* logoImage = new GUIImage("Textures/logo.png", screenWidth / 2, screenHeight - 100.f, 1.4f, VAO, VBO, GUIShader);

	GUIButton* startButton = new GUIButton("Start", screenWidth / 2, 250, 1,
		darkGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
	GUIManager::MainMenu_StartButtonClick = startButton;

	GUIButton* optionsButton = new GUIButton("Options", screenWidth / 2, 175, 1,
		darkGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
	GUIManager::MainMenu_OptionsButtonClick = optionsButton;

	GUIButton* quitButton = new GUIButton("Quit", screenWidth / 2, 100, 1,
		darkGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
	GUIManager::MainMenu_QuitButtonClick = quitButton;

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
	std::vector<std::string> noteText = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };

	for (int i = 1; i < 13; i++) {
		float percentHeight = static_cast<float>(i) / 13.f;
		float textHeight = percentHeight * screenHeight;

		GUIButton* tempText = new GUIButton(noteText[i - 1], screenWidth - 50.f, textHeight, .4f,
			 white, 
			 white,
			 nullptr, *arialFont, VAO, VBO, GUIShader);
		gameGUIVector.push_back(tempText);
	}

	GUIButton* scoreGUI = new GUIButton("0", screenWidth / 2, screenHeight - 100, 1.f, 
		white, 
		white,
		nullptr, *arialFont, VAO, VBO, GUIShader);
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
	GUIObject* background = new GUIImage("Textures/holder.png", screenWidth / 2, screenHeight / 2, 10, VAO, VBO, GUIShader);

	
	GUIButton* backButton = new GUIButton("Back", screenWidth / 2, 100.f, 1,
		darkGray,
		lightGray,
		[this] {
		showMainMenu();
	}, *arialFont, VAO, VBO, GUIShader);
	
	GUIButton* yourScoreText = new GUIButton("Your Score:", screenWidth / 2.f, 340.f, 1.2f,
		lightGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);

	GUIButton* scoreScreenText = new GUIButton("0", screenWidth / 2, 250.f, 1,
		lightGray,
		lightGray,
		nullptr, *arialFont, VAO, VBO, GUIShader);
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

