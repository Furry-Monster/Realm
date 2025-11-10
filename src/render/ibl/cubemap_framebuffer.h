#pragma once

namespace RealmEngine
{
    /**
     * Framebuffer for rendering to faces of a cubemap.
     */
    class CubemapFramebuffer
    {
    public:
        CubemapFramebuffer(int width, int height);
        void bind();
        void generateMipmap();

        /**
         * Set which cube face texture to render to.
         * @param index
         */
        void setCubeFace(unsigned int index);

        unsigned int getCubemapTextureId() const;

    private:
        unsigned int  m_framebuffer_id;
        unsigned int  m_depth_renderbuffer_id;
        unsigned int  m_cubemap_texture_id;
    };
} // namespace RealmEngine

