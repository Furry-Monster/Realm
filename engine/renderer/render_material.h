#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "renderer/texture.h"

namespace RealmEngine
{
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

    struct RenderMaterial
    {
        bool use_texture_albedo             = false;
        bool use_texture_metallic_roughness = false;
        bool use_texture_normal             = false;
        bool use_texture_ambient_occlusion  = false;
        bool use_texture_emissive           = false;

        glm::vec3 albedo            = glm::vec3(1.0f, 0.0f, 0.0f);
        float     metallic          = 1.0f;
        float     roughness         = 0.0f;
        float     ambient_occlusion = 1.0f;
        glm::vec3 emissive          = glm::vec3(0.0, 0.0, 0.0);

        std::shared_ptr<Texture> texture_albedo;
        std::shared_ptr<Texture> texture_metallic_roughness;
        std::shared_ptr<Texture> texture_normal;
        std::shared_ptr<Texture> texture_ambient_occlusion;
        std::shared_ptr<Texture> texture_emissive;
    };
} // namespace RealmEngine
