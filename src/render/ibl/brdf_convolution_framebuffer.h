#pragma once

namespace RealmEngine
{
    class BrdfConvolutionFramebuffer
    {
    public:
        BrdfConvolutionFramebuffer(unsigned int width, unsigned int height);

        void bind() const;

        unsigned int getWidth() const;
        unsigned int getHeight() const;
        unsigned int getColorTextureId() const;

    private:
        unsigned int m_width, m_height;

        unsigned int m_framebuffer_id;
        unsigned int m_depth_renderbuffer_id;
        unsigned int m_color_texture_id;
    };
} // namespace RealmEngine
