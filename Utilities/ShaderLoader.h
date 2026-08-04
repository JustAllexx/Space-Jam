#pragma once

#include <GL/glew.h>
#include <GL/glut.h>
#include <string>
#include <fstream>

class Shader {
private:
    GLuint shaderID;
    static std::string readShaderFile(const char* filename) {
        std::string ret;
        std::fstream shaderFile(filename);
        std::string nextLine;
        while (getline(shaderFile, nextLine)) {
            ret += nextLine + "\n";
        }
        return ret;
    };
public:
    Shader(GLenum shaderType, const char* shaderPath);
    ~Shader() {
        glDeleteShader(shaderID);
    };
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    GLuint getShaderID() const noexcept {return shaderID;}
};

class Program {
private:
    Shader vertexShader;
    Shader fragShader;
    GLuint programID;
public:
    Program(const char* vertexPath, const char* fragPath);
    ~Program() {
        glDeleteProgram(programID);
    };
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    GLuint getVertexShaderID() const noexcept {return vertexShader.getShaderID();}
    GLuint getFragmentShaderID() const noexcept {return fragShader.getShaderID();}
    GLuint getProgramID() const noexcept {return programID;}
};