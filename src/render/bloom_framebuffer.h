#pragma once

namespace RealmEngine
{
    class BloomFramebuffer
    {
    public:
        BloomFramebuffer(int width, int height);
        void         init();
        void         bind() const;
        void         resize(int width, int height);
        void         setMipLevel(int mipLevel);
        unsigned int getColorTextureId() const;

    private:
        int          m_width, m_height;
        int          m_mip_level = 0;
        unsigned int m_framebuffer_id;
        unsigned int m_color_texture_id;
    };
} // namespace RealmEngine
