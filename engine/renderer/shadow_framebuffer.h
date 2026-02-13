#pragma once

namespace RealmEngine
{
    static constexpr int SHADOW_WIDTH  = 2048;
    static constexpr int SHADOW_HEIGHT = 2048;

    class ShadowFramebuffer
    {

    public:
        ShadowFramebuffer(int width, int height);
        ~ShadowFramebuffer() noexcept;

        ShadowFramebuffer(const ShadowFramebuffer&)            = delete;
        ShadowFramebuffer& operator=(const ShadowFramebuffer&) = delete;
        ShadowFramebuffer(ShadowFramebuffer&&) noexcept;
        ShadowFramebuffer& operator=(ShadowFramebuffer&&) noexcept;

        void init();
        void bind() const;
        void resize(int width, int height);

        unsigned int getFramebufferId() const;
        unsigned int getDepthTextureId() const;

    private:
        int          m_width, m_height;
        unsigned int m_framebuffer;
        unsigned int m_depth_texture;
    };
} // namespace RealmEngine
