#include "GUIButton.h"
#include "TypeChar.h"
#include "Utilities/FontLoader.h"
#include <stdexcept>

GUIButton::GUIButton(std::string inText, float inX, float inY, float inScale,
	 glm::vec3 inColour, glm::vec3 inHoverColour, 
	 std::function<void()> callback, SJ_Font& inFontMap,
	 GLuint inVAO, GLuint inVBO, GLuint inGUIShader)
	  : GUIObject(inGUIShader), VAO(inVAO), VBO(inVBO), rFontMap(inFontMap){
	{
		//Sets input variables as class variables
		x = inX; y = inY; scale = inScale;
		text = inText;
		if (callback != nullptr) { //Blank case, if function is equal to nullptr, don't assign it to onClick
			onClick = callback;
		}

		colour[0] = inColour.r; colour[1] = inColour.g; colour[2] = inColour.b;
		hoverColour[0] = inHoverColour.r; hoverColour[1] = inHoverColour.g; hoverColour[2] = inHoverColour.b;

		//Advance Sum calculates how big the text is based on the "advance" and "scale" of each character loaded in by the font loader
		advanceSum = 0;
		maxHeight = 0;
		//Iterates through every character in a string
		std::string::const_iterator tempIt;
		const auto fontMap = rFontMap.getMap();
		for (tempIt = text.begin(); tempIt != text.end(); tempIt++) {
			TypeChar tempChar = fontMap.at(*tempIt);
			advanceSum += static_cast<float>(tempChar.Advance >> 6) * scale;
			//Gets the maximum height by comparing the height of every character in the text
			maxHeight = std::max(maxHeight, tempChar.Size.y * scale);
		}
		advanceSum = advanceSum / 2;

		//Clickable Directions (Coordinates for where the click region starts and ends based on the text
		float rightBound = x + advanceSum;
		float leftBound = x - advanceSum;
		if (rightBound > static_cast<float>(std::numeric_limits<int>::max()) ||
			leftBound < static_cast<float>(std::numeric_limits<int>::min())) {
			throw std::runtime_error("Button horizontal bounds too big, overflowed/underflowed the integer limit");
		}
		right = static_cast<int>(rightBound);
		left = static_cast<int>(leftBound);

		float topBound = inY + std::ceil(maxHeight/2);
		float bottomBound = inY - std::floor(maxHeight/2);
		if (topBound > static_cast<float>(std::numeric_limits<int>::max()) ||
			bottomBound < static_cast<float>(std::numeric_limits<int>::min())) {
			throw std::runtime_error("Button vertical bounds too big, overflowed/underflowed the integer limit");
		}
		top = static_cast<int>(topBound);
		bottom = static_cast<int>(bottomBound);
		
		//Enable the button
		enable = true;
	}
}

//Override for the ObjectGUI Render
//Renders each character in a string, renders each character indivudally 
void GUIButton::Render() {
	//Tell the GUI Shader Programme that I am rendering text
	glProgramUniform1i(GUIShader, isTextPos, 1);
	//If the text is being hovered over, set it to the hover colour, if not to the deafult colour
	if (hovered) {
		glUniform3f(glGetUniformLocation(GUIShader, "textColor"), hoverColour[0], hoverColour[1], hoverColour[2]);
	}
	else {
		glUniform3f(glGetUniformLocation(GUIShader, "textColor"), colour[0], colour[1], colour[2]);
	}

	if (!enable) { //If the button is not enabled don't render it
		return;
	}
	//Binds the information for drawing quads
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	float charx = x;

	std::string::const_iterator c;
	const auto fontMap = rFontMap.getMap();
	for (c = text.begin(); c != text.end(); c++)
	{
		//The character struct for each character being rendered
		TypeChar ch = fontMap.at(*c);

		float xpos = (charx - advanceSum) + ch.Bearing.x * scale;
		float ypos = (y - (maxHeight / 2)) - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// the Quad coordinates for each character being rendered
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		charx += static_cast<float>(ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	//Clear the vertex array and the texture once finished rendering
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}