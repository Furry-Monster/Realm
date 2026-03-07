#pragma once

#include <cstdint>

namespace RealmEngine
{
    enum class ViewportMode : uint8_t
    {
        Scene,
        Game
    };

    enum class ViewportDisplayMode : uint16_t
    {
        Lit = 0,
        Albedo,
        Normals,
        Metallic,
        Roughness,
        MaterialAO,
        Emissive,
        AO,
        Depth,
        SSR,
        Count
    };
} // namespace RealmEngine
