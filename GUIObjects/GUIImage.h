#pragma once

#include "GUIObject.h"

//The class for displaying Images, is a GUI element and so inherits from the GUI Element class
class GUIImage : public GUIObject {
private:
	//Properties about the image, location ID of image texture
	float posX, posY, scale;
	int imgHeight, imgWidth;
	GLuint texture, VAO, VBO;
public:
	//Constructor function for image class
	GUIImage(const char* imagePath, float inX, float inY, float inScale, GLuint inVAO, GLuint inVBO, GLuint inGUIShader);
	void Render(); //Override for GUIObject Render Function
};