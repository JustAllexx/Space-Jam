#pragma once

#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <unordered_map>

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
    inline static GLuint currentProgram{0};

    Shader vertexShader;
    Shader fragShader;
    GLuint programID;
    std::unordered_map<std::string, GLuint> uniformLocations;

    GLuint getUniformLocation(std::string uniformLoc) {
        auto it = uniformLocations.find(uniformLoc);
        if (it != uniformLocations.end()) {return it->second;}
        GLuint loc = glGetUniformLocation(programID, uniformLoc.c_str());
        uniformLocations[uniformLoc] = loc;
        return loc;
    }

public:
    Program(const char* vertexPath, const char* fragPath);
    ~Program() {
        glDeleteProgram(programID);
    };
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    void use() {
        //Not all of the code uses Programs yet so this condition is not trivially true
        //if (currentProgram != programID) {
            glUseProgram(programID);
            currentProgram = programID;
        //}
    }

    GLuint getVertexShaderID() const noexcept {return vertexShader.getShaderID();}
    GLuint getFragmentShaderID() const noexcept {return fragShader.getShaderID();}
    GLuint getProgramID() const noexcept {return programID;}

    void setMat4(std::string uniformLoc, glm::mat4 mat);
    void setFloat(std::string uniformLoc, GLfloat value); 
    void setInt(std::string uniformLoc, GLint value);
};