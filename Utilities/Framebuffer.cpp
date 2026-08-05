#include "Utilities/Framebuffer.h"
#include <Utilities/ObjectLoader.h>

Framebuffer::Framebuffer(GLint colourAttachment) : texture(std::make_unique<Texture>()) {
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(1, framebufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, colourAttachment, GL_TEXTURE_2D, texture->getTextureID(), 0);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &framebufferID);
}