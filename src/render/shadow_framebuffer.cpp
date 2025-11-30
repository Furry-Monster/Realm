#include "render/shadow_framebuffer.h"

#include <glad/gl.h>
#include "utils.h"

namespace RealmEngine
{
    ShadowFramebuffer::ShadowFramebuffer(int width, int height) : m_width(width), m_height(height) {}

    ShadowFramebuffer::~ShadowFramebuffer() noexcept
    {
        if (m_framebuffer != 0)
            glDeleteFramebuffers(1, &m_framebuffer);

        if (m_depth_texture != 0)
            glDeleteTextures(1, &m_depth_texture);
    }

    ShadowFramebuffer::ShadowFramebuffer(ShadowFramebuffer&& other) noexcept :
        m_width(other.m_width), m_height(other.m_height), m_framebuffer(other.m_framebuffer),
        m_depth_texture(other.m_depth_texture)
    {
        other.m_framebuffer   = 0;
        other.m_depth_texture = 0;
    }

    ShadowFramebuffer& ShadowFramebuffer::operator=(ShadowFramebuffer&& other) noexcept
    {
        if (this != &other)
        {
            if (m_framebuffer != 0)
                glDeleteFramebuffers(1, &m_framebuffer);

            if (m_depth_texture != 0)
                glDeleteTextures(1, &m_depth_texture);

            m_width         = other.m_width;
            m_height        = other.m_height;
            m_framebuffer   = other.m_framebuffer;
            m_depth_texture = other.m_depth_texture;

            other.m_framebuffer   = 0;
            other.m_depth_texture = 0;
        }

        return *this;
    }

    void ShadowFramebuffer::init()
    {
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        // create depth texture
        glGenTextures(1, &m_depth_texture);
        glBindTexture(GL_TEXTURE_2D, m_depth_texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);

        // NOTE:
        // don't draw to color buffer, read and write are not needed
        // we simply banned them here.
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fatal("Error initializing shadow framebuffer: framebuffer not complete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ShadowFramebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
        glViewport(0, 0, m_width, m_height);
    }

    void ShadowFramebuffer::resize(int width, int height)
    {
        m_width  = width;
        m_height = height;

        glBindTexture(GL_TEXTURE_2D, m_depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    unsigned int ShadowFramebuffer::getFramebufferId() const { return m_framebuffer; }

    unsigned int ShadowFramebuffer::getDepthTextureId() const { return m_depth_texture; }
} // namespace RealmEngine
