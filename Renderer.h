#include "optional"
#include "Utilities/ShaderLoader.h"
#include "Utilities/Framebuffer.h"

class GameManager;

class Renderer {
private:
    std::optional<Program> shaderProgram;
    std::optional<Program> guiProgram;
    std::optional<Program> screenProgram;
    std::optional<Program> gaussianProgram;

    std::optional<Framebuffer> renderFramebuffer;
    std::optional<Framebuffer> gaussianHorizontalBuffer;
    std::optional<Framebuffer> gaussianVerticalBuffer;
public:
    void displayFrameBuffers();
    void createFrameBuffers();
    void render(GameManager& gameManager);

    Renderer();
    ~Renderer();
};