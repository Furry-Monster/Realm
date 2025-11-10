#include "render/ibl/cubemap_framebuffer.h"

#include <glad/gl.h>

namespace RealmEngine
{
    CubemapFramebuffer::CubemapFramebuffer(int width, int height)
    {
        // framebuffer
        glGenFramebuffers(1, &mFramebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);

        // depth buffer
        glGenRenderbuffers(1, &mDepthRenderbufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

        // attach the depth buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderbufferId);

        // cubemap
        glGenTextures(1, &mCubemapTextureId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapTextureId);

        // specify/allocate each face for the cubemap
        for (auto i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         GL_RGB16F,
                         width,
                         height,
                         0,
                         GL_RGB,
                         GL_FLOAT,
                         nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void CubemapFramebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId); }

    void CubemapFramebuffer::generateMipmap()
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapTextureId);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    void CubemapFramebuffer::setCubeFace(unsigned int index)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + index,
                                mCubemapTextureId,
                                0);
    }

    unsigned int CubemapFramebuffer::getCubemapTextureId() const { return mCubemapTextureId; }
} // namespace RealmEngine

