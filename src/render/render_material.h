#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "render/texture.h"

namespace RealmEngine
{
    // texture channel offsets
    const static int TEXTURE_UNIT_ALBEDO                 = 0; // pbr
    const static int TEXTURE_UNIT_METALLIC_ROUGHNESS     = 1;
    const static int TEXTURE_UNIT_NORMAL                 = 2;
    const static int TEXTURE_UNIT_AMBIENT_OCCLUSION      = 3;
    const static int TEXTURE_UNIT_EMISSIVE               = 4;
    const static int TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP = 10; // ibl
    const static int TEXTURE_UNIT_PREFILTERED_ENV_MAP    = 11;
    const static int TEXTURE_UNIT_BRDF_CONVOLUTION_MAP   = 12;
    const static int TEXTURE_UNIT_SHADOW_MAP             = 13; // shadow

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
