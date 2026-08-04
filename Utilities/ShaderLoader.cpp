#include "Utilities/ShaderLoader.h"
#include <GUIObjects/TypeChar.h>
#include <stdexcept>

Shader::Shader(GLenum shaderType, const char* shaderPath) : 
 shaderID(glCreateShader(shaderType)){
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
    programID(glCreateProgram()) {
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

        glUseProgram(programID);
    }