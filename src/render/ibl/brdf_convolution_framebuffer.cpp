#include "render/ibl/brdf_convolution_framebuffer.h"

#include <glad/gl.h>

namespace RealmEngine
{
    BrdfConvolutionFramebuffer::BrdfConvolutionFramebuffer(unsigned int width, unsigned int height) :
        m_width(width), m_height(height)
    {
        // framebuffer
        glGenFramebuffers(1, &m_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

        // depth buffer
        glGenRenderbuffers(1, &m_depth_renderbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_renderbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

        // attach the depth buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_renderbuffer_id);

        // color texture
        glGenTextures(1, &m_color_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_color_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture_id, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BrdfConvolutionFramebuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id); }

    unsigned int BrdfConvolutionFramebuffer::getWidth() const { return m_width; }

    unsigned int BrdfConvolutionFramebuffer::getHeight() const { return m_height; }

    unsigned int BrdfConvolutionFramebuffer::getColorTextureId() const { return m_color_texture_id; }
} // namespace RealmEngine
