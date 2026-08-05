#pragma once

#include "Utilities/ObjectLoader.h"
#include <memory>

class Framebuffer {
private:
    GLuint framebufferID;
    std::unique_ptr<Texture> texture;
public:
    Framebuffer(GLint colourAttachment);
    ~Framebuffer();
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
};