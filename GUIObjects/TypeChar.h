#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/glut.h>

//The GUI Object is the parent class of all GUI Elements
//Contains the overridable function render, this is what is called by the GUIManager when rendering a frame
//Most GUI elements will override this element, so by default it returns nothing
//A C++ Structure, stores details about every character that can be rendered to the screen
//Every character in a font will have these details
struct TypeChar {
	GLuint TextureID;
	glm::vec2 Size;
	glm::vec2 Bearing;
	unsigned int Advance;
};