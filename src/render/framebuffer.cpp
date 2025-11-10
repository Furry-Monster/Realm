#include "render/framebuffer.h"

#include "utils.h"
#include <glad/gl.h>

namespace RealmEngine
{
    Framebuffer::Framebuffer(int width, int height) : mWidth(width), mHeight(height) {}

    void Framebuffer::init()
    {
        // create the framebuffer
        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        // create color texture
        glGenTextures(1, &mColorTexture);
        glBindTexture(GL_TEXTURE_2D, mColorTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTexture, 0);

        // create bloom texture
        glGenTextures(1, &mBloomColorTexture);
        glBindTexture(GL_TEXTURE_2D, mBloomColorTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mBloomColorTexture, 0);

        glBindTexture(GL_TEXTURE_2D, 0);

        // create depth/stencil buffer
        // we use renderbuffer which is similar to textures except you can't sample it
        // that is okay because we never actually need to read the values in depth/stencil buffer
        // its all done by hardware depth/stencil tests
        glGenRenderbuffers(1, &mDepthStencilRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // attach the renderbuffer to the framebuffer
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRenderbuffer);

        unsigned int color_attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, color_attachments);

        // make sure the framebuffer was created successfully
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            err("Error initializing framebuffer: framebuffer not complete");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::bind() const{ glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer); }

    void Framebuffer::resize(int width, int height)
    {
        mWidth  = width;
        mHeight = height;

        // resize color textures
        glBindTexture(GL_TEXTURE_2D, mColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, mBloomColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        // resize renderbuffer
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    unsigned int Framebuffer::getFramebufferId() const { return mFramebuffer; }

    unsigned int Framebuffer::getColorTextureId() const { return mColorTexture; }

    unsigned int Framebuffer::getBloomColorTextureId() const { return mBloomColorTexture; }
} // namespace RealmEngine
