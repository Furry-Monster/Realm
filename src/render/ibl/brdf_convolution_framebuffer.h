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
        void bind() const;

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
        unsigned int m_width, m_height;

        unsigned int m_framebuffer_id;
        unsigned int m_depth_renderbuffer_id;
        unsigned int m_color_texture_id;
    };
} // namespace RealmEngine
