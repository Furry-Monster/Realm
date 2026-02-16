#pragma once

#include <cstdint>
#include <string>

#include "renderer/material_property_block.h"

namespace RealmEngine
{
    // Texture unit bindings (shared convention for standard shaders)
    static constexpr int TEXTURE_UNIT_ALBEDO                 = 0;
    static constexpr int TEXTURE_UNIT_METALLIC_ROUGHNESS     = 1;
    static constexpr int TEXTURE_UNIT_NORMAL                 = 2;
    static constexpr int TEXTURE_UNIT_AMBIENT_OCCLUSION      = 3;
    static constexpr int TEXTURE_UNIT_EMISSIVE               = 4;
    static constexpr int TEXTURE_UNIT_OPACITY                = 5;
    static constexpr int TEXTURE_UNIT_CUSTOM_BASE            = 6;
    static constexpr int TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP = 10;
    static constexpr int TEXTURE_UNIT_PREFILTERED_ENV_MAP    = 11;
    static constexpr int TEXTURE_UNIT_BRDF_CONVOLUTION_MAP   = 12;
    static constexpr int TEXTURE_UNIT_SHADOW_MAP             = 13;
    static constexpr int TEXTURE_UNIT_SSAO_DEPTH             = 14;
    static constexpr int TEXTURE_UNIT_SSAO_NOISE             = 15;
    static constexpr int TEXTURE_UNIT_SSAO_RESULT            = 16;

    enum class ShadingModel : uint8_t
    {
        StandardPBR,
        Unlit,
        Custom
    };

    enum class BlendMode : uint8_t
    {
        Opaque,
        AlphaTest,
        Transparent
    };

    enum class RenderFace : uint8_t
    {
        Front,
        Back,
        Both
    };

    class Material
    {
    public:
        std::string name;

        ShadingModel shading_model = ShadingModel::StandardPBR;
        BlendMode    blend_mode    = BlendMode::Opaque;
        RenderFace   render_face   = RenderFace::Front;
        float        alpha_cutoff  = 0.5f;
        int          render_queue  = 2000;

        std::string vert_path;
        std::string frag_path;

        MaterialPropertyBlock properties;

        [[nodiscard]] bool isOpaque() const
        {
            return blend_mode == BlendMode::Opaque || blend_mode == BlendMode::AlphaTest;
        }

        [[nodiscard]] bool isTransparent() const { return blend_mode == BlendMode::Transparent; }

        [[nodiscard]] bool isDoubleSided() const { return render_face == RenderFace::Both; }

        [[nodiscard]] bool hasCustomShader() const { return !vert_path.empty() && !frag_path.empty(); }

        // Deferred-compatible: only standard PBR opaque materials
        [[nodiscard]] bool isDeferred() const
        {
            return shading_model == ShadingModel::StandardPBR && isOpaque();
        }
    };

} // namespace RealmEngine
