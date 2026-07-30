//pragma once means "include once", if this file is accidentally included somewhere other than the main file it should be ignored
//This is in every header file
#pragma once

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
class buttonGUI;


std::string readShaderFile(std::string filename);

//The OptionsManager is in charge of changing values when the user changes a setting in the Options Menu
//This class also defines all the button behaviours for every GUI Element on the screen
class OptionsManager {
private:
	static size_t samplesOptionIndex;
	static buttonGUI* samplesGUI;
	static std::vector<std::string> samplesOptionsText;
public:
	static void Initialise();
	static void IncrementSamplesOption();
	static void DecrementSamplesOption();
};

