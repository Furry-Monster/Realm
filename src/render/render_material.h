#pragma once

#include "render/graphic_res_manager.h"
#include "resource/datatype/model/material.h"
#include <optional>

namespace RealmEngine
{
    class RenderMaterial
    {
    public:
        RenderMaterial()  = default;
        ~RenderMaterial() = default;

        RenderMaterial(const RenderMaterial& that)                = delete;
        RenderMaterial& operator=(const RenderMaterial& that)     = delete;
        RenderMaterial(RenderMaterial&& that) noexcept            = default;
        RenderMaterial& operator=(RenderMaterial&& that) noexcept = default;

        // Texture handles
        void setBaseColorTexture(TextureHandle handle);
        void setMetallicRoughnessTexture(TextureHandle handle);
        void setNormalTexture(TextureHandle handle);
        void setOcclusionTexture(TextureHandle handle);
        void setEmissiveTexture(TextureHandle handle);

        std::optional<TextureHandle> getBaseColorTexture() const;
        std::optional<TextureHandle> getMetallicRoughnessTexture() const;
        std::optional<TextureHandle> getNormalTexture() const;
        std::optional<TextureHandle> getOcclusionTexture() const;
        std::optional<TextureHandle> getEmissiveTexture() const;

        // Render state
        void               setRenderState(const RenderState& state);
        const RenderState& getRenderState() const;

    private:
        // GPU texture handles
        std::optional<TextureHandle> m_base_color_texture;
        std::optional<TextureHandle> m_metallic_roughness_texture;
        std::optional<TextureHandle> m_normal_texture;
        std::optional<TextureHandle> m_occlusion_texture;
        std::optional<TextureHandle> m_emissive_texture;

        RenderState m_render_state;
    };
} // namespace RealmEngine
