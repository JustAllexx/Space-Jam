#include "Utilities/Framebuffer.h"
#include <Utilities/ObjectLoader.h>

Framebuffer::Framebuffer(int numAttachments) : texture0(std::make_unique<Texture>()) {
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(1, framebufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture0->getTextureID(), 0);
    if (numAttachments == 2) {
        texture1.emplace();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture1->getTextureID(), 0);
    }
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &framebufferID);
}