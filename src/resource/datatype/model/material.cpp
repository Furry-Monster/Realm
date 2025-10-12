#include "material.h"

namespace RealmEngine
{
    // Texture
    void Material::setBaseColorTexture(const std::string& path) { m_base_color_texture = TextureRef {path, 0, true}; }
    void Material::setMetallicRoughnessTexture(const std::string& path)
    {
        m_metallic_roughness_texture = TextureRef {path, 1, false};
    }
    void Material::setNormalTexture(const std::string& path) { m_normal_texture = TextureRef {path, 2, false}; }
    void Material::setOcclusionTexture(const std::string& path) { m_occlusion_texture = TextureRef {path, 3, false}; }
    void Material::setEmissiveTexture(const std::string& path) { m_emissive_texture = TextureRef {path, 4, true}; }

    const std::optional<TextureRef>& Material::getBaseColorTexture() const { return m_base_color_texture; }
    const std::optional<TextureRef>& Material::getMetallicRoughnessTexture() const
    {
        return m_metallic_roughness_texture;
    }
    const std::optional<TextureRef>& Material::getNormalTexture() const { return m_normal_texture; }
    const std::optional<TextureRef>& Material::getOcclusionTexture() const { return m_occlusion_texture; }
    const std::optional<TextureRef>& Material::getEmissiveTexture() const { return m_emissive_texture; }

    // Material property
    void Material::setBaseColorFactor(const glm::vec4& color) { m_base_color_factor = color; }
    void Material::setMetallicFactor(float metallic) { m_metallic_factor = metallic; }
    void Material::setRoughnessFactor(float roughness) { m_roughness_factor = roughness; }
    void Material::setEmissiveFactor(const glm::vec3& emissive) { m_emissive_factor = emissive; }
    void Material::setNormalScale(float normal_scale) { m_normal_scale = normal_scale; }
    void Material::setOcclusionStrength(float occlusion_strength) { m_occlusion_strength = occlusion_strength; }

    const glm::vec4& Material::getBaseColorFactor() const { return m_base_color_factor; }
    float            Material::getMetallicFactor() const { return m_metallic_factor; }
    float            Material::getRoughnessFactor() const { return m_roughness_factor; }
    const glm::vec3& Material::getEmissiveFactor() const { return m_emissive_factor; }
    float            Material::getNormalScale() const { return m_normal_scale; }
    float            Material::getOcclusionStrength() const { return m_occlusion_strength; }

    // Render state
    void               Material::setRenderState(const RenderState& state) { m_render_state = state; }
    const RenderState& Material::getRenderState() const { return m_render_state; }

} // namespace RealmEngine