#include "render/ibl/mipmap_cubemap_framebuffer.h"

#include <cmath>
#include <glad/gl.h>

namespace RealmEngine
{
    MipmapCubemapFramebuffer::MipmapCubemapFramebuffer(unsigned int width, unsigned int height) :
        m_width(width), m_height(height), m_mip_level(0)
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

        // cubemap
        glGenTextures(1, &m_cubemap_texture_id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture_id);

        // specify/allocate each face for the cubemap
        for (auto i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(
            GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trilinear filtering for the mipmap
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void MipmapCubemapFramebuffer::bind()const { glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id); }

    void MipmapCubemapFramebuffer::setMipLevel(unsigned int level)
    {
        m_mip_level  = level;
        m_mip_width  = static_cast<unsigned int>(m_width * std::pow(0.5, m_mip_level));
        m_mip_height = static_cast<unsigned int>(m_height * std::pow(0.5, m_mip_level));

        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_renderbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_mip_width, m_mip_height);
    }

    unsigned int MipmapCubemapFramebuffer::getWidth() const { return m_mip_width; }

    unsigned int MipmapCubemapFramebuffer::getHeight() const { return m_mip_height; }

    void MipmapCubemapFramebuffer::setCubeFace(unsigned int faceIndex)const
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex,
                               m_cubemap_texture_id,
                               m_mip_level);
    }

    unsigned int MipmapCubemapFramebuffer::getCubemapTextureId() const { return m_cubemap_texture_id; }
} // namespace RealmEngine
