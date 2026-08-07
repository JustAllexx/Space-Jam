#pragma once

#include "Utilities/ObjectLoader.h"
#include <memory>
#include <optional>
#include <stdexcept>

class Framebuffer {
private:
    GLuint framebufferID;
    std::unique_ptr<Texture> texture0;
    std::optional<Texture> texture1;
public:
    Framebuffer(int numAttachments);
    ~Framebuffer();
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    void bind() const {glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);}
    static void bindRenderFramebuffer() {glBindFramebuffer(GL_FRAMEBUFFER, 0);}
    GLuint getAttachment0ID() const {return texture0->getTextureID();}
    GLuint getAttachment1ID() const {
        if (texture1.has_value()) {
            return texture1->getTextureID();
        } else {
            throw std::runtime_error("Attempted to request Attachment 1, but framebuffer was created with only Attachment 0");
        }
    }
};