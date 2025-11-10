#pragma once

namespace RealmEngine
{
    /**
     * Framebuffer for rendering brdf convolution map.
     */
    class BrdfConvolutionFramebuffer
    {
    public:
        BrdfConvolutionFramebuffer(unsigned int width, unsigned int height);

        /**
         * Activate this framebuffer for drawing.
         */
        void bind();

        /**
         * Get the width.
         * @return
         */
        unsigned int getWidth() const;

        /**
         * Get the height.
         * @return
         */
        unsigned int getHeight() const;

        /**
         * Get the GL color texture id.
         * @return
         */
        unsigned int getColorTextureId() const;

    private:
        unsigned int mWidth, mHeight;

        unsigned int mFramebufferId;
        unsigned int mDepthRenderbufferId;
        unsigned int mColorTextureId;
    };
} // namespace RealmEngine

