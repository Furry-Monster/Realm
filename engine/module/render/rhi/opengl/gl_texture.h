#pragma once

#include <cstdint>

#include "module/render/rhi/rhi_texture.h"

namespace RealmEngine
{
    class GLTexture final : public RHITexture
    {
    public:
        explicit GLTexture(const TextureDesc& desc);

        // Adopt an existing GL texture id (for IBL / legacy interop)
        GLTexture(uint32_t native_id, TextureType type, TextureFormat format, int width, int height);

        ~GLTexture() noexcept override;

        GLTexture(const GLTexture&)            = delete;
        GLTexture& operator=(const GLTexture&) = delete;

        void bind(uint32_t unit = 0) override;
        void unbind(uint32_t unit = 0) override;
        void generateMipmaps() override;
        void resize(int width, int height) override;
        void bindImage(uint32_t unit, int level, bool layered, TextureAccess access) override;

        uint32_t getNativeHandle() const override { return m_id; }

    private:
        uint32_t m_id {0};
        uint32_t m_target {0};   // GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP
        bool     m_owned {true}; // false if adopted (don't delete in dtor)
    };

} // namespace RealmEngine
