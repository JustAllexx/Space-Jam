#pragma once
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

//The class that is reponsible for loading wavefront files and buffering textures into OpenGL
class ObjectLoader
{
public:
    static void loadOBJ(
        const char* path,
        std::vector < glm::vec3 >& out_vertices,
        std::vector < glm::vec2 >& out_uvs,
        std::vector < glm::vec3 >& out_normals
    );

    static GLuint loadTexture(
        const char* path
    );
};

class STBIImage {
private:
    int imageWidth{0};
    int imageHeight{0};
    int numChannels{0};
    stbi_uc* data{0};
public:
    STBIImage(const char* filepath) {
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(filepath, &imageWidth, &imageHeight, &numChannels, 0);
    }
    ~STBIImage() {
        stbi_image_free(data);
    }
    STBIImage(const STBIImage&) = delete;
    STBIImage& operator=(const STBIImage&) = delete;

    int getImageWidth() const noexcept {return imageWidth;}
    int getImageHeight() const noexcept {return imageHeight;}
    int getChannels() const noexcept {return numChannels;}
    stbi_uc* getData() const noexcept {return data;}
};

class Texture {
private:
    GLuint textureID{0};
public:
    Texture(const char* filepath);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    GLuint getTextureID() const noexcept {return textureID;}
};
