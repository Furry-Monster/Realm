#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace RealmEngine
{
    class RHITexture;

    // Texture unit bindings
    static constexpr int TEXTURE_UNIT_ALBEDO                 = 0; // PBR material textures
    static constexpr int TEXTURE_UNIT_METALLIC_ROUGHNESS     = 1;
    static constexpr int TEXTURE_UNIT_NORMAL                 = 2;
    static constexpr int TEXTURE_UNIT_AMBIENT_OCCLUSION      = 3;
    static constexpr int TEXTURE_UNIT_EMISSIVE               = 4;
    static constexpr int TEXTURE_UNIT_OPACITY                = 5;
    static constexpr int TEXTURE_UNIT_CUSTOM_BASE            = 6; // Custom shader textures start here
    static constexpr int TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP = 10; // IBL
    static constexpr int TEXTURE_UNIT_PREFILTERED_ENV_MAP    = 11;
    static constexpr int TEXTURE_UNIT_BRDF_CONVOLUTION_MAP   = 12;
    static constexpr int TEXTURE_UNIT_SHADOW_MAP             = 13; // Shadow
    static constexpr int TEXTURE_UNIT_SSAO_DEPTH             = 14;
    static constexpr int TEXTURE_UNIT_SSAO_NOISE             = 15;
    static constexpr int TEXTURE_UNIT_SSAO_RESULT            = 16;

    // ---- Custom shader material parameter types ----

    enum class MaterialParamType : uint8_t
    {
        Float,
        Int,
        Vec2,
        Vec3,
        Vec4,
        Color3, // vec3 displayed as color picker in editor
        Color4  // vec4 displayed as color picker in editor
    };

    struct MaterialParam
    {
        std::string       name;
        MaterialParamType type = MaterialParamType::Float;
        float             values[4] {};
    };

    struct RenderMaterial
    {
        std::string name;

        bool use_texture_albedo             = false;
        bool use_texture_opacity            = false;
        bool use_texture_metallic_roughness = false;
        bool use_texture_normal             = false;
        bool use_texture_ambient_occlusion  = false;
        bool use_texture_emissive           = false;

        glm::vec3 albedo            = glm::vec3(0.7f, 0.7f, 0.7f);
        float     opacity           = 1.0f;
        float     alpha_cutout      = 0.5f;
        float     metallic          = 0.0f;
        float     roughness         = 0.5f;
        float     ambient_occlusion = 1.0f;
        glm::vec3 emissive          = glm::vec3(0.0, 0.0, 0.0);
        float     emissive_strength = 1.0f;

        bool      subsurface_enabled = false;
        float     subsurface_radius  = 1.0f;
        glm::vec3 subsurface_color   = glm::vec3(1.0f, 0.2f, 0.1f);

        bool  is_transparent         = false;
        bool  double_sided           = false;
        bool  is_hair                = false;
        int   hair_layers            = 8;
        float hair_layer_step        = 0.002f;
        float hair_specular_strength = 0.5f;
        float hair_specular_power    = 64.0f;

        // Custom shader
        bool        use_custom_shader = false;
        std::string custom_vert_path;
        std::string custom_frag_path;

        std::vector<MaterialParam> custom_params;

        std::shared_ptr<RHITexture> texture_albedo;
        std::shared_ptr<RHITexture> texture_opacity;
        std::shared_ptr<RHITexture> texture_metallic_roughness;
        std::shared_ptr<RHITexture> texture_normal;
        std::shared_ptr<RHITexture> texture_ambient_occlusion;
        std::shared_ptr<RHITexture> texture_emissive;

        bool hasCustomShader() const
        {
            return use_custom_shader && !custom_vert_path.empty() && !custom_frag_path.empty();
        }
    };
} // namespace RealmEngine
