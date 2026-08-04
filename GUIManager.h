#pragma once

#include "FontLoader.h"
#include "GUIObjects/TypeChar.h"

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <memory>
#include <optional>
#include <vector>
#include <ft2build.h>
#include <unordered_map>

//Forward declare
class GUIObject;
class Clickable;
class GUIButton;

//GUIManager controls all the GUI Elements rendered onto the screen
class GUIManager
{
private:
	std::vector<GUIObject*> guiRenderQueue;
	std::vector<Clickable*> clickChecks;
	std::unordered_map<char, TypeChar> fontMap;
	std::unique_ptr<SJ_Font> arialFont;
	GLuint VAO, VBO;
	GLuint GUIShader, isTextPos;
public:
	GUIManager(GLuint program);

	void Setup();
	void renderQueue(); //Called to render GUI onscreen
	void checkCollisions(int mousePosX, int mousePosY, bool clicked); //Called to check if any element on the screen has been cliked

	//Inititates all the Screens that are used in the programme, every GUIObject neeeds to exist to define its behaviour if clicked
	void createOptionsMenu();
	void showOptionsMenu();

	void createMainMenu();
	void showMainMenu();

	void createGameGUI();
	void showGameGUI();

	void createScoreMenu();
	void showScoreMenu();

	//Game Menu
	std::vector<GUIObject*> gameGUIVector;
	std::vector<Clickable*> gameGUIClickables;

	//Options Menu
	std::vector<GUIObject*> optionsMenuVector;
	std::vector<Clickable*> optionsMenuClickables;
	//Main Menu
	std::vector<GUIObject*> mainMenuVector;
	std::vector<Clickable*> mainMenuClickables;
	//Final Score Menu
	std::vector<GUIObject*> scoreMenuVector;
	std::vector<Clickable*> scoreMenuClickables;

	//Options Menu Buttons
	GUIButton* samplesOptionLeftClick{nullptr};
	GUIButton* samplesOptionRightClick{nullptr};
	GUIButton* samplesOptionText{nullptr};
	GUIButton* samplesBackClick{nullptr};
	//Main Menu Buttons
	GUIButton* MainMenu_StartButtonClick{nullptr};
	GUIButton* MainMenu_OptionsButtonClick{nullptr};
	GUIButton* MainMenu_QuitButtonClick{nullptr};
	//Game Menu Buttons
	GUIButton* GameGUI_ScoreText{nullptr};
	//Final score buttons
	GUIButton* scoreScreen_FinalScoreText{nullptr};

};


