#pragma once

namespace RealmEngine
{
    class Framebuffer
    {
        // this is the main frame buffer
    public:
        Framebuffer(int width, int height);

        void init();
        void bind() const;
        void resize(int width, int height);

        unsigned int getFramebufferId() const;
        unsigned int getColorTextureId() const;
        unsigned int getBloomColorTextureId() const;

    private:
        int          m_width, m_height;
        unsigned int m_framebuffer;
        unsigned int m_color_texture;
        unsigned int m_bloom_color_texture;
        unsigned int m_depth_stencil_renderbuffer;
    };
} // namespace RealmEngine
