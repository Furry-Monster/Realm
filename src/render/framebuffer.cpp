#include "render/framebuffer.h"

#include "utils.h"
#include <glad/gl.h>

namespace RealmEngine
{
    Framebuffer::Framebuffer(int width, int height) : m_width(width), m_height(height) {}

    void Framebuffer::init()
    {
        // create the framebuffer
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        // create color texture
        glGenTextures(1, &m_color_texture);
        glBindTexture(GL_TEXTURE_2D, m_color_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

        // create bloom texture
        glGenTextures(1, &m_bloom_color_texture);
        glBindTexture(GL_TEXTURE_2D, m_bloom_color_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_bloom_color_texture, 0);

        glBindTexture(GL_TEXTURE_2D, 0);

        // create depth/stencil buffer
        // we use renderbuffer which is similar to textures except you can't sample it
        // that is okay because we never actually need to read the values in depth/stencil buffer
        // its all done by hardware depth/stencil tests
        glGenRenderbuffers(1, &m_depth_stencil_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // attach the renderbuffer to the framebuffer
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_stencil_renderbuffer);

        unsigned int color_attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, color_attachments);

        // make sure the framebuffer was created successfully
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            err("Error initializing framebuffer: framebuffer not complete");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer); }

    void Framebuffer::resize(int width, int height)
    {
        m_width  = width;
        m_height = height;

        // resize color textures
        glBindTexture(GL_TEXTURE_2D, m_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, m_bloom_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        // resize renderbuffer
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    unsigned int Framebuffer::getFramebufferId() const { return m_framebuffer; }

    unsigned int Framebuffer::getColorTextureId() const { return m_color_texture; }

    unsigned int Framebuffer::getBloomColorTextureId() const { return m_bloom_color_texture; }
} // namespace RealmEngine
