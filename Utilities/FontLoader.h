#pragma once

#include "GUIObjects/TypeChar.h"
#include <freetype/freetype.h>
#include <stdexcept>
#include <unordered_map>
#include FT_FREETYPE_H

//RAII wrapper that holds the freetype library objects and the font face
class FreeTypeHandle {
private:
    FT_Library library;
public:
    FT_Face face;

    FreeTypeHandle(const char* fontpath) {
        if (FT_Init_FreeType(&library)) {throw std::runtime_error("Failed to initialise freetype");}
        if (FT_New_Face(library, fontpath, 0, &face)) {throw std::runtime_error("Failed to open font");}
    };
    ~FreeTypeHandle() {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    };
    FreeTypeHandle(const FreeTypeHandle&) = delete;
    FreeTypeHandle& operator=(const FreeTypeHandle&) = delete;
};

class SJ_Font {
private:
    std::unordered_map<char, TypeChar> fontMap;
public:
    SJ_Font(const char* fontpath);
    ~SJ_Font();
    SJ_Font(const SJ_Font&) = delete;
    SJ_Font& operator=(const SJ_Font&) = delete;

    const std::unordered_map<char, TypeChar>& getMap() const noexcept {return fontMap;};
};