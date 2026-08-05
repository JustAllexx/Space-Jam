#pragma once

#include "GUIObject.h"
#include <Utilities/ShaderLoader.h>
#include <string>
#include <glm/glm.hpp>

//TODO: Maybe fix this, I don't know if I am allowed to do this
struct TypeChar;
class SJ_Font;
class Program;

//The button class inherits from the GUIObject class because it is an onscreen element, it also inherits from the Clickable class because it can be clicked
class GUIButton : public GUIObject, public Clickable {
private:
	//Properties about the text being rendered (advanceSum and maxHeight are used to calculate how big the click region should be)
	float advanceSum, x, y, scale, maxHeight;
	glm::vec3 colour;
	glm::vec3 hoverColour;
	GLuint VAO, VBO;
	SJ_Font& rFontMap;
public:
	//If a button is not enabled it will not be rendered, is enabled by default in the constructor, text stores the text that is rendered
	bool enable;
	std::string text;
	//The constructor for the buttonGUI Class
	GUIButton(std::string inText, float inX, float inY, float inScale, 
		glm::vec3 inColour, glm::vec3 inHoverColour, 
		std::function<void()> callbackFunc, SJ_Font& inFontMap,
		GLuint inVAO, GLuint inVBO, Program& GUIShader);
	void Render(); //Override for the GUIObject Render Function
};