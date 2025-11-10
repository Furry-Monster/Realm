#pragma once

namespace RealmEngine
{
    /**
     * The main framebuffer.
     */
    class Framebuffer
    {
    public:
        Framebuffer(int width, int height);
        void         init();
        void         bind() const;
        void         resize(int width, int height);
        unsigned int getFramebufferId() const;
        unsigned int getColorTextureId() const;
        unsigned int getBloomColorTextureId() const;

    private:
        int          mWidth, mHeight;
        unsigned int mFramebuffer;
        unsigned int mColorTexture;
        unsigned int mBloomColorTexture;
        unsigned int mDepthStencilRenderbuffer;
    };
} // namespace RealmEngine
