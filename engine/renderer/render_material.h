#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace RealmEngine
{
    class RHITexture;

    // Texture unit bindings
    static constexpr int TEXTURE_UNIT_ALBEDO                 = 0; // PBR material textures
    static constexpr int TEXTURE_UNIT_METALLIC_ROUGHNESS     = 1;
    static constexpr int TEXTURE_UNIT_NORMAL                 = 2;
    static constexpr int TEXTURE_UNIT_AMBIENT_OCCLUSION      = 3;
    static constexpr int TEXTURE_UNIT_EMISSIVE               = 4;
    static constexpr int TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP = 10; // IBL
    static constexpr int TEXTURE_UNIT_PREFILTERED_ENV_MAP    = 11;
    static constexpr int TEXTURE_UNIT_BRDF_CONVOLUTION_MAP   = 12;
    static constexpr int TEXTURE_UNIT_SHADOW_MAP             = 13; // Shadow
    static constexpr int TEXTURE_UNIT_SSAO_DEPTH             = 14;
    static constexpr int TEXTURE_UNIT_SSAO_NOISE             = 15;
    static constexpr int TEXTURE_UNIT_SSAO_RESULT            = 16;

    struct RenderMaterial
    {
        bool use_texture_albedo             = false;
        bool use_texture_metallic_roughness = false;
        bool use_texture_normal             = false;
        bool use_texture_ambient_occlusion  = false;
        bool use_texture_emissive           = false;

        glm::vec3 albedo            = glm::vec3(0.7f, 0.7f, 0.7f);
        float     metallic          = 0.0f;
        float     roughness         = 0.5f;
        float     ambient_occlusion = 1.0f;
        glm::vec3 emissive          = glm::vec3(0.0, 0.0, 0.0);

        bool      subsurface_enabled = false;
        float     subsurface_radius  = 1.0f;
        glm::vec3 subsurface_color   = glm::vec3(1.0f, 0.2f, 0.1f);

        std::shared_ptr<RHITexture> texture_albedo;
        std::shared_ptr<RHITexture> texture_metallic_roughness;
        std::shared_ptr<RHITexture> texture_normal;
        std::shared_ptr<RHITexture> texture_ambient_occlusion;
        std::shared_ptr<RHITexture> texture_emissive;
    };
} // namespace RealmEngine
