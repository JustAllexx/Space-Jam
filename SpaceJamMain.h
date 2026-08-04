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

