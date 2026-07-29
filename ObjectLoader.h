#pragma once
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

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

class Mesh {
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    GLuint vertexBuffer, normalBuffer, uvBuffer;
    GLuint vertexArrayID;
public:
    Mesh(const char* filepath);
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    const std::vector<glm::vec3>& getVertices() const noexcept {return vertices;}
    const std::vector<glm::vec3>& getNormals() const noexcept {return normals;}
    const std::vector<glm::vec2>& getUVs() const noexcept {return uvs;}
    GLuint getVertexBuffer() const noexcept {return vertexBuffer;}
    GLuint getNormalBuffer() const noexcept {return normalBuffer;}
    GLuint getUVBuffer() const noexcept {return uvBuffer;}
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
