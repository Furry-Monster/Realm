#include "render/bloom_framebuffer.h"

#include <cmath>
#include <glad/gl.h>

namespace RealmEngine
{
    BloomFramebuffer::BloomFramebuffer(int width, int height) : mWidth(width), mHeight(height) {}

    void BloomFramebuffer::init()
    {
        // create the framebuffer
        glGenFramebuffers(1, &mFramebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);

        // create a color texture
        glGenTextures(1, &mColorTextureId);
        glBindTexture(GL_TEXTURE_2D, mColorTextureId);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_2D);

        // attach the color texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTextureId, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BloomFramebuffer::bind()
    {
        int width  = static_cast<int>(mWidth / std::pow(2, mMipLevel));
        int height = static_cast<int>(mHeight / std::pow(2, mMipLevel));

        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);
    }

    void BloomFramebuffer::resize(int width, int height)
    {
        mWidth  = width;
        mHeight = height;
        // resize color textures
        glBindTexture(GL_TEXTURE_2D, mColorTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void BloomFramebuffer::setMipLevel(int mipLevel)
    {
        mMipLevel = mipLevel;
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTextureId, mipLevel);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int BloomFramebuffer::getColorTextureId() const { return mColorTextureId; }
} // namespace RealmEngine

