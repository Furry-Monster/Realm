#include "render/ibl/brdf_convolution_framebuffer.h"

#include <glad/gl.h>

namespace RealmEngine
{
    BrdfConvolutionFramebuffer::BrdfConvolutionFramebuffer(unsigned int width, unsigned int height)
        : mWidth(width), mHeight(height)
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

        // color texture
        glGenTextures(1, &mColorTextureId);
        glBindTexture(GL_TEXTURE_2D, mColorTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTextureId, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BrdfConvolutionFramebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId); }

    unsigned int BrdfConvolutionFramebuffer::getWidth() const { return mWidth; }

    unsigned int BrdfConvolutionFramebuffer::getHeight() const { return mHeight; }

    unsigned int BrdfConvolutionFramebuffer::getColorTextureId() const { return mColorTextureId; }
} // namespace RealmEngine

