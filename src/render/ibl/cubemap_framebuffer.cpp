#include "render/ibl/cubemap_framebuffer.h"

#include <glad/gl.h>

namespace RealmEngine
{
    CubemapFramebuffer::CubemapFramebuffer(int width, int height)
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
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void CubemapFramebuffer::bind()const { glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id); }

    void CubemapFramebuffer::generateMipmap()const
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture_id);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    void CubemapFramebuffer::setCubeFace(unsigned int index) const
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, m_cubemap_texture_id, 0);
    }

    unsigned int CubemapFramebuffer::getCubemapTextureId() const { return m_cubemap_texture_id; }
} // namespace RealmEngine
