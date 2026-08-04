#include "GUIImage.h"
#include <stb/stb_image.h>


GUIImage::GUIImage(const char* imagePath, float inX, float inY, float inScale, GLuint inVAO, GLuint inVBO, GLuint inGUIShader) : GUIObject(inGUIShader) {
	posX = inX; posY = inY; scale = inScale;
	VAO = inVAO; VBO = inVBO;

	stbi_set_flip_vertically_on_load(false);
	int numChannels;
	unsigned char* bytes = stbi_load(imagePath, &imgWidth, &imgHeight, &numChannels, 0);

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(bytes);
}

//Render Function for ImagGUI class, an override for the GUIObject class, very similar to rendering a quad for a character, but calculates the coordinates of the quad slightly differently
void GUIImage::Render() {
	glDisable(GL_DEPTH_TEST);
	//Tell the GUI Program shader that I'm not rendering text and it should render an image
	glProgramUniform1i(GUIShader, isTextPos, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	float h = static_cast<float>(imgHeight) * scale * 0.5f;
	float w = static_cast<float>(imgWidth) * scale * 0.5f;

	float vertices[6][4] = {
			{ posX - w,     posY + h,   0.0f, 0.0f },
			{ posX - w,     posY - h,       0.0f, 1.0f },
			{ posX + w , posY - h,       1.0f, 1.0f },

			{ posX - w,     posY + h,   0.0f, 0.0f },
			{ posX + w, posY - h,       1.0f, 1.0f },
			{ posX + w, posY + h,   1.0f, 0.0f }
	};

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// render quad
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}