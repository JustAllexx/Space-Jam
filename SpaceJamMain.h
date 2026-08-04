//pragma once means "include once", if this file is accidentally included somewhere other than the main file it should be ignored
//This is in every header file
#pragma once

#include "GUIManager.h"
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <stb/stb_image.h>

//Forward Delcare GUI objects
class GUIButton;
class GameManager;

std::string readShaderFile(std::string filename);

//The OptionsManager is in charge of changing values when the user changes a setting in the Options Menu
//This class also defines all the button behaviours for every GUI Element on the screen
class OptionsManager {
private:
	GUIManager& guiManager;
	GUIButton& samplesGUI;
	GameManager* gameManager;

	size_t samplesOptionIndex{0};
	std::vector<std::string> samplesOptionsText{{
		"1024 Samples",
		"2048 Samples",
		"4096 Samples",
		"512 Samples"
	}};
public:
	OptionsManager(GUIManager& guiManager, GameManager* gameManager);
	void Initialise();
	void IncrementSamplesOption();
	void DecrementSamplesOption();
};

