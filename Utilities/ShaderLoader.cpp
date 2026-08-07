#include "Utilities/ShaderLoader.h"
#include <GUIObjects/TypeChar.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <stdexcept>
#include <iostream>

Shader::Shader(GLenum shaderType, const char* shaderPath) : 
    shaderID(glCreateShader(shaderType))
{
    const std::string shaderCode = readShaderFile(shaderPath);
    const GLchar* shaderBuffer = shaderCode.c_str();
    glShaderSource(shaderID, 1, &shaderBuffer, NULL);
    glCompileShader(shaderID);

    GLint bCompiled;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &bCompiled);
    if (!bCompiled) {
        GLint logLength;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
        size_t logSize = static_cast<size_t>(logLength) + 1;
        GLchar* log = new GLchar[logSize];
        glGetShaderInfoLog(shaderID, logLength, &logLength, log);
        throw std::runtime_error(log);
    }
}

Program::Program(const char* vertexPath, const char* fragPath) :
    vertexShader(GL_VERTEX_SHADER, vertexPath),
    fragShader(GL_FRAGMENT_SHADER, fragPath),
    programID(glCreateProgram()) 
{
    glAttachShader(programID, vertexShader.getShaderID());
    glAttachShader(programID, fragShader.getShaderID());
    GLint bLinked;
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &bLinked);
    if (!bLinked) {
        GLint logLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
        size_t logSize = static_cast<size_t>(logLength) + 1;
        GLchar* log = new GLchar[logSize];
        glGetProgramInfoLog(programID, logLength, &logLength, log);
        throw std::runtime_error(log);
    } 
}

void Program::setFloat(std::string uniformLoc, GLfloat value) {
    use();
    GLint loc = getUniformLocation(uniformLoc);
    glUniform1f(loc, value);
}

void Program::setInt(std::string uniformLoc, GLint value) {
    use();
    GLint loc = getUniformLocation(uniformLoc);
    glUniform1i(loc, value);
}

void Program::setMat4(std::string uniformLoc, glm::mat4 value) {
    use();
    GLint loc = getUniformLocation(uniformLoc);
    glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
}

void Program::setVec3(std::string uniformLoc, glm::vec3 value) {
    use();
    GLint loc = getUniformLocation(uniformLoc);
    glUniform3f(loc, value.x, value.y, value.z);
}