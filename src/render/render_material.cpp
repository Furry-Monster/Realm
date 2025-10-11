#include "render_material.h"

namespace RealmEngine
{
    // Texture setters
    void RenderMaterial::setBaseColorTexture(TextureHandle handle) { m_base_color_texture = handle; }
    void RenderMaterial::setMetallicRoughnessTexture(TextureHandle handle) { m_metallic_roughness_texture = handle; }
    void RenderMaterial::setNormalTexture(TextureHandle handle) { m_normal_texture = handle; }
    void RenderMaterial::setOcclusionTexture(TextureHandle handle) { m_occlusion_texture = handle; }
    void RenderMaterial::setEmissiveTexture(TextureHandle handle) { m_emissive_texture = handle; }

    // Texture getters
    std::optional<TextureHandle> RenderMaterial::getBaseColorTexture() const { return m_base_color_texture; }
    std::optional<TextureHandle> RenderMaterial::getMetallicRoughnessTexture() const
    {
        return m_metallic_roughness_texture;
    }
    std::optional<TextureHandle> RenderMaterial::getNormalTexture() const { return m_normal_texture; }
    std::optional<TextureHandle> RenderMaterial::getOcclusionTexture() const { return m_occlusion_texture; }
    std::optional<TextureHandle> RenderMaterial::getEmissiveTexture() const { return m_emissive_texture; }

    // Render state
    void               RenderMaterial::setRenderState(const RenderState& state) { m_render_state = state; }
    const RenderState& RenderMaterial::getRenderState() const { return m_render_state; }
} // namespace RealmEngine