#include "GUIImage.h"
#include <stb/stb_image.h>

std::array<std::array<float, 4>, 6> GUIImage::createVertexData(float x, float y, float size, int height, int width) {
	float h = static_cast<float>(height) * size * 0.5f;
	float w = static_cast<float>(width) * size * 0.5f;
	return {{
			{ x - w,     y + h,   0.0f, 0.0f },
			{ x - w,     y - h,       0.0f, 1.0f },
			{ x + w ,    y - h,       1.0f, 1.0f },

			{ x - w,     y + h,   0.0f, 0.0f },
			{ x + w,     y - h,       1.0f, 1.0f },
			{ x + w,     y + h,   1.0f, 0.0f }
	}};
}

//Texture flip needs to be false here
GUIImage::GUIImage(const char* imagePath, float inX, float inY, float inScale, GLuint inVAO, GLuint inVBO, GLuint inGUIShader) 
: GUIObject(inGUIShader), posX(inX),
  posY(inY), scale(inScale), texture(imagePath, false),
  imgHeight(texture.getImageHeight()), imgWidth(texture.getImageWidth()), VAO(inVAO), VBO(inVBO),
  vertexInfo(createVertexData(posX, posY, scale, imgHeight, imgWidth)) {
}

//Render Function for ImagGUI class, an override for the GUIObject class, very similar to rendering a quad for a character, but calculates the coordinates of the quad slightly differently
void GUIImage::Render() {
	glDisable(GL_DEPTH_TEST);
	//Tell the GUI Program shader that I'm not rendering text and it should render an image
	glProgramUniform1i(GUIShader, isTextPos, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	glBindTexture(GL_TEXTURE_2D, texture.getTextureID());
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexInfo), vertexInfo.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// render quad
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}