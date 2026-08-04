#pragma once

#include "GUIObject.h"
#include "Utilities/ObjectLoader.h"
#include <array>

//The class for displaying Images, is a GUI element and so inherits from the GUI Element class
class GUIImage : public GUIObject {
private:
	static constexpr std::array<std::array<float, 4>, 6> createVertexData(float x, float y, float size, int height, int width);

	//Properties about the image, location ID of image texture
	float posX, posY, scale;
	Texture texture;
	int imgHeight, imgWidth;
	GLuint VAO, VBO;

	//Vertex information for drawing
	std::array<std::array<float, 4>, 6> vertexInfo;
public:
	//Constructor function for image class
	GUIImage(const char* imagePath, float inX, float inY, float inScale, GLuint inVAO, GLuint inVBO, GLuint inGUIShader);
	void Render(); //Override for GUIObject Render Function
};