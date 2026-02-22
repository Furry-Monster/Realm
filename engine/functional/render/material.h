#pragma once

#include <cstdint>
#include <string>

#include "functional/render/material_property_block.h"

namespace RealmEngine
{
    inline constexpr int TEXTURE_UNIT_ALBEDO                 = 0;
    inline constexpr int TEXTURE_UNIT_METALLIC_ROUGHNESS     = 1;
    inline constexpr int TEXTURE_UNIT_NORMAL                 = 2;
    inline constexpr int TEXTURE_UNIT_AMBIENT_OCCLUSION      = 3;
    inline constexpr int TEXTURE_UNIT_EMISSIVE               = 4;
    inline constexpr int TEXTURE_UNIT_OPACITY                = 5;
    inline constexpr int TEXTURE_UNIT_CUSTOM_BASE            = 6;
    inline constexpr int TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP = 10;
    inline constexpr int TEXTURE_UNIT_PREFILTERED_ENV_MAP    = 11;
    inline constexpr int TEXTURE_UNIT_BRDF_CONVOLUTION_MAP   = 12;
    inline constexpr int TEXTURE_UNIT_SHADOW_MAP             = 13;
    inline constexpr int TEXTURE_UNIT_GTAO_DEPTH             = 14;
    inline constexpr int TEXTURE_UNIT_GTAO_NOISE             = 15;
    inline constexpr int TEXTURE_UNIT_GTAO_RESULT            = 16;

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

        // Deferred-compatible: standard PBR opaque materials without
        // special forward-only rendering requirements.
        [[nodiscard]] bool isDeferred() const
        {
            if (shading_model != ShadingModel::StandardPBR || !isOpaque())
                return false;
            if (hasCustomShader())
                return false;
            // Hair requires multi-layer forward rendering
            if (properties.getBool("isHair"))
                return false;
            // SSS parameters can't be encoded in the current G-Buffer
            if (properties.getBool("material.subsurfaceEnabled"))
                return false;
            return true;
        }
    };

} // namespace RealmEngine
