#include "render/bloom_framebuffer.h"

#include <cmath>
#include <glad/gl.h>

namespace RealmEngine
{
    BloomFramebuffer::BloomFramebuffer(int width, int height) : m_width(width), m_height(height) {}

    void BloomFramebuffer::init()
    {
        // create the framebuffer
        glGenFramebuffers(1, &m_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

        // create a color texture
        glGenTextures(1, &m_color_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_color_texture_id);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_2D);

        // attach the color texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture_id, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BloomFramebuffer::bind()
    {
        int width  = static_cast<int>(m_width / std::pow(2, m_mip_level));
        int height = static_cast<int>(m_height / std::pow(2, m_mip_level));

        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
    }

    void BloomFramebuffer::resize(int width, int height)
    {
        m_width  = width;
        m_height = height;
        // resize color textures
        glBindTexture(GL_TEXTURE_2D, m_color_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void BloomFramebuffer::setMipLevel(int mipLevel)
    {
        m_mip_level = mipLevel;
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture_id, mipLevel);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int BloomFramebuffer::getColorTextureId() const { return m_color_texture_id; }
} // namespace RealmEngine

