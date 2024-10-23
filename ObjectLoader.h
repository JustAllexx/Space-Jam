#pragma once
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include <string>
#include <fstream>
#include <sstream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

//The class that is reponsible for loading wavefront files and buffering textures into OpenGL
class ObjectLoader
{
public:
    static bool loadOBJ(
        const char* path,
        std::vector < glm::vec3 >& out_vertices,
        std::vector < glm::vec2 >& out_uvs,
        std::vector < glm::vec3 >& out_normals
    );

    static GLuint loadTexture(
        const char* path
    );
};

