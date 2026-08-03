#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <ft2build.h>
#include <map>

#include <stb/stb_image.h>

//The GUI Object is the parent class of all GUI Elements
//Contains the overridable function render, this is what is called by the GUIManager when rendering a frame
//Most GUI elements will override this element, so by default it returns nothing
class GUIObject {
public:
	virtual void Render() {
		return;
	}
};

//The class which describes elements on the screen that are clickable by a mouse
//When check collission is called this all members of this class are iterated through to see if the click was within the region
class Clickable {
protected:
	// Describes the shape of the click region (from the centre)
	int top, bottom, left, right;
public:
	bool hovered; //Is the Element being hovered over
	bool checkCollision(int mousePosX, int mousePosY); //Check if a collission has occured
	//Each clickable object contains 2 function pointers, these pointers contain the memory location of a function, these functions are called whenever the button is clicked or hovered over
	//Setter functions for the Click function pointer and the hover function pointer
	void setClickFunction(void(*newClickFunction)());
	void setHoverFunction(void(*newHoverFunction)());
	//Function pointers
	void (*onHover)();
	void (*onClick)();
};

//The button class inherits from the GUIObject class because it is an onscreen element, it also inherits from the Clickable class because it can be clicked
class buttonGUI : public GUIObject, public Clickable {
private:
	//Properties about the text being rendered (advanceSum and maxHeight are used to calculate how big the click region should be)
	float advanceSum, x, y, scale, maxHeight;
	GLfloat colour[3];
	GLfloat hoverColour[3];
public:
	//If a button is not enabled it will not be rendered, is enabled by default in the constructor, text stores the text that is rendered
	bool enable;
	std::string text;
	//The constructor for the buttonGUI Class
	buttonGUI(std::string inText, float inX, float inY, float inScale, GLfloat colR, GLfloat colG, GLfloat colB, GLfloat hovR, GLfloat hovG, GLfloat hovB, void (*f)());
	void Render(); //Override for the GUIObject Render Function
};

//The class for displaying Images, is a GUI element and so inherits from the GUI Element class
class imageGUI : public GUIObject {
private:
	//Properties about the image, location ID of image texture
	float posX, posY, scale;
	int imgHeight, imgWidth;
	GLuint texture;
public:
	//Constructor function for image class
	imageGUI(const char* imagePath, float inX, float inY, float inScale);
	void Render(); //Override for GUIObject Render Function
};

//GUIManager controls all the GUI Elements rendered onto the screen
class GUIManager
{
public:
	GUIManager();

	void Setup(GLuint program);
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
	buttonGUI* samplesOptionLeftClick{nullptr};
	buttonGUI* samplesOptionRightClick{nullptr};
	buttonGUI* samplesOptionText{nullptr};
	buttonGUI* samplesBackClick{nullptr};
	//Main Menu Buttons
	buttonGUI* MainMenu_StartButtonClick{nullptr};
	buttonGUI* MainMenu_OptionsButtonClick{nullptr};
	buttonGUI* MainMenu_QuitButtonClick{nullptr};
	//Game Menu Buttons
	buttonGUI* GameGUI_ScoreText{nullptr};
	//Final score buttons
	buttonGUI* scoreScreen_FinalScoreText{nullptr};

};


