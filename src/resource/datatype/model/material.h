#pragma once

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include <cstdint>
#include <optional>
#include <string>

namespace RealmEngine
{
    struct TextureRef
    {
        std::string path;
        uint32_t    slot {0};
        bool        srgb {true};
    };

    struct RenderState
    {
        enum class BlendMode : uint8_t
        {
            Opaque,
            AlphaBlend,
            Additive,
            Multiply,
        };

        enum class CullMode : uint8_t
        {
            None,
            Front,
            Back,
        };

        enum class DepthTest : uint8_t
        {
            Always,
            Less,
            LessEqual,
            Greater,
            Equal,
        };

        BlendMode blend_mode {BlendMode::Opaque};
        CullMode  cull_mode {CullMode::Back};
        DepthTest depth_test {DepthTest::Less};
        bool      depth_write {true};
    };

    class Material
    {
    public:
        Material()           = default;
        ~Material() noexcept = default;

        Material(const Material& that)                = delete;
        Material& operator=(const Material& that)     = delete;
        Material(Material&& that) noexcept            = default;
        Material& operator=(Material&& that) noexcept = default;

        // Texture
        void setBaseColorTexture(const std::string& path);
        void setMetallicRoughnessTexture(const std::string& path);
        void setNormalTexture(const std::string& path);
        void setOcclusionTexture(const std::string& path);
        void setEmissiveTexture(const std::string& path);

        const std::optional<TextureRef>& getBaseColorTexture() const;
        const std::optional<TextureRef>& getMetallicRoughnessTexture() const;
        const std::optional<TextureRef>& getNormalTexture() const;
        const std::optional<TextureRef>& getOcclusionTexture() const;
        const std::optional<TextureRef>& getEmissiveTexture() const;

        // Material properties
        void setBaseColorFactor(const glm::vec4& color);
        void setMetallicFactor(float metallic);
        void setRoughnessFactor(float roughness);
        void setEmissiveFactor(const glm::vec3& emissive);
        void setNormalScale(float normal_scale);
        void setOcclusionStrength(float occlusion_strength);

        const glm::vec4& getBaseColorFactor() const;
        float            getMetallicFactor() const;
        float            getRoughnessFactor() const;
        const glm::vec3& getEmissiveFactor() const;
        float            getNormalScale() const;
        float            getOcclusionStrength() const;

        // Render state
        void               setRenderState(const RenderState& state);
        const RenderState& getRenderState() const;

    private:
        // PBR textures
        std::optional<TextureRef> m_base_color_texture;
        std::optional<TextureRef> m_metallic_roughness_texture;
        std::optional<TextureRef> m_normal_texture;
        std::optional<TextureRef> m_occlusion_texture;
        std::optional<TextureRef> m_emissive_texture;

        // PBR factors
        glm::vec4 m_base_color_factor {1.0f, 1.0f, 1.0f, 1.0f};
        float     m_metallic_factor {1.0f};
        float     m_roughness_factor {1.0f};
        glm::vec3 m_emissive_factor {0.0f, 0.0f, 0.0f};
        float     m_normal_scale {1.0f};
        float     m_occlusion_strength {1.0f};

        // General render state
        RenderState m_render_state;
    };
} // namespace RealmEngine