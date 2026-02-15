#pragma once

namespace RealmEngine
{
    enum class ViewportDisplayMode : int
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
