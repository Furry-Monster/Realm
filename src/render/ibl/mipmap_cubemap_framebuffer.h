#pragma once

namespace RealmEngine
{
    /**
     * Framebuffer for rendering to faces of a cubemap. The faces themselves are mipmapped.
     */
    class MipmapCubemapFramebuffer
    {
    public:
        MipmapCubemapFramebuffer(unsigned int width, unsigned int height);
        void bind();

        /**
         * Set the mip level to render with.
         * @param mipLevel
         */
        void setMipLevel(unsigned int level);

        /**
         * Get the current width based on the mip level.
         * @return
         */
        unsigned int getWidth() const;

        /**
         * Get the current height based on the mip level.
         * @return
         */
        unsigned int getHeight() const;

        /**
         * Set which cube face texture to render to.
         * @param index
         */
        void setCubeFace(unsigned int faceIndex);

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

