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
        unsigned int mWidth, mHeight;
        unsigned int mMipWidth, mMipHeight;
        unsigned int mMipLevel;

        unsigned int mFramebufferId;
        unsigned int mDepthRenderbufferId;
        unsigned int mCubemapTextureId;
    };
} // namespace RealmEngine

