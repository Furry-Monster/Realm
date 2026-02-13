#pragma once

namespace RealmEngine
{
    class MipmapCubemapFramebuffer
    {
    public:
        MipmapCubemapFramebuffer(unsigned int width, unsigned int height);
        void bind() const;

        void setMipLevel(unsigned int level);
        void setCubeFace(unsigned int faceIndex) const;

        unsigned int getWidth() const;
        unsigned int getHeight() const;
        unsigned int getCubemapTextureId() const;

    private:
        unsigned int m_width, m_height;
        unsigned int m_mip_width, m_mip_height;
        unsigned int m_mip_level;

        unsigned int m_framebuffer_id;
        unsigned int m_depth_renderbuffer_id;
        unsigned int m_cubemap_texture_id;
    };
} // namespace RealmEngine
