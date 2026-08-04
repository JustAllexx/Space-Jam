#include "FontLoader.h"
#include <stdexcept>

SJ_Font::SJ_Font(const char* fontpath) {
    FreeTypeHandle ft(fontpath);
    FT_Face& face = ft.face;
    FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	//Iterates over the unicode characters from 0-127, gives the basic ASCII character set
	for (unsigned int c = 0; c < 128; c++) {
		//Uses the FT library to load character for specific character
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) { throw std::runtime_error("Failed to load glyph"); }

		//Generate the texture for this specific character
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RED, //Characters don't have colour by default, so only need 1 Colour channel
			face->glyph->bitmap.width, //Width of the specific character (or glyph)
			face->glyph->bitmap.rows, //Height of the character
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer //byte content of the image of the character (rendered similarly to an image)
		);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		//Define the values for a structure 
		TypeChar character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			(unsigned int) face->glyph->advance.x
		};
		//Insert the struct into a map so that properties about the character being rendered can be requested on rendering
		fontMap.insert(std::pair<char, TypeChar>(c, character));
	}
}

SJ_Font::~SJ_Font() {};