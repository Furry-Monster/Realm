#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace RealmEngine
{
    class AssetManager;
    class RHIDevice;
    class RHITexture;
    class RenderMesh;

    class ModelLoader
    {
    public:
        static std::vector<RenderMesh>
        load(const std::string& path, bool flip_textures, RHIDevice& device, AssetManager* asset_mgr = nullptr);
    };

} // namespace RealmEngine
