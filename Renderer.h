#pragma once

#include <optional>
#include "Utilities/ShaderLoader.h"
#include "Utilities/Framebuffer.h"

class GameManager;

class Renderer {
private:
    constexpr static float vertices[6][4] = {
        {-1.f, 1.f, 0.f, 1.f},
        {-1.f, -1.f, 0.f, 0.f},
        {1.f, -1.f, 1.f, 0.f},

        {-1.f, 1.f, 0.f, 1.f},
        {1.f, -1.f, 1.f, 0.f},
        {1.f, 1.f, 1.f, 1.f}
    };

    std::optional<Program> shaderProgram;
    std::optional<Program> guiProgram;
    std::optional<Program> screenProgram;
    std::optional<Program> gaussianProgram;

    std::optional<Framebuffer> renderFramebuffer;
    std::optional<Framebuffer> gaussianHorizontalBuffer;
    std::optional<Framebuffer> gaussianVerticalBuffer;

    GLuint RBO;
    GLuint screenVAO, screenVBO;

    glm::mat4 projection, modelview; 
public:
    void displayFrameBuffer();
    void createFrameBuffers();
    void createPrograms();
    void render(GameManager& gameManager);

    Program& getShaderProgram() {return shaderProgram.value();}
    Program& getGUIProgram() {return guiProgram.value();}

    Renderer();
    ~Renderer();
};