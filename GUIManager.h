#include "AudioManager.h";

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <ft2build.h>
#include <map>

#include <stb/stb_image.h>

#pragma once
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
	static void Setup(GLuint program);
	static void renderQueue(); //Called to render GUI onscreen
	static void checkCollisions(int mousePosX, int mousePosY, bool clicked); //Called to check if any element on the screen has been cliked

	//Inititates all the Screens that are used in the programme, every GUIObject neeeds to exist to define its behaviour if clicked
	static void createOptionsMenu();
	static void showOptionsMenu();

	static void createMainMenu();
	static void showMainMenu();

	static void createGameGUI();
	static void showGameGUI();

	static void createScoreMenu();
	static void showScoreMenu();

	//Game Menu
	static std::vector<GUIObject*> gameGUIVector;
	static std::vector<Clickable*> gameGUIClickables;

	//Options Menu
	static std::vector<GUIObject*> optionsMenuVector;
	static std::vector<Clickable*> optionsMenuClickables;
	//Main Menu
	static std::vector<GUIObject*> mainMenuVector;
	static std::vector<Clickable*> mainMenuClickables;
	//Final Score Menu
	static std::vector<GUIObject*> scoreMenuVector;
	static std::vector<Clickable*> scoreMenuClickables;

	//Options Menu Buttons
	static buttonGUI* samplesOptionLeftClick;
	static buttonGUI* samplesOptionRightClick;
	static buttonGUI* samplesOptionText;
	static buttonGUI* samplesBackClick;
	//Main Menu Buttons
	static buttonGUI* MainMenu_StartButtonClick;
	static buttonGUI* MainMenu_OptionsButtonClick;
	static buttonGUI* MainMenu_QuitButtonClick;
	//Game Menu Buttons
	static buttonGUI* GameGUI_ScoreText;
	//Final score buttons
	static buttonGUI* scoreScreen_FinalScoreText;

};


