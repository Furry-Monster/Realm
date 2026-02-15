#pragma once

#include <cstdint>

namespace RealmEngine
{
    enum class ViewportDisplayMode : uint16_t
    {
        Lit = 0,
        Albedo,
        Normals,
        Metallic,
        Roughness,
        MaterialAO,
        Emissive,
        SSAO,
        Depth,
        Count
    };
} // namespace RealmEngine
