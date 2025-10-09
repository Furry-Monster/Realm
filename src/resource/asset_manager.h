#pragma once

#include "resource/material.h"
#include "resource/mesh.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace RealmEngine
{
    class AssetManager
    {
    public:
        AssetManager()           = default;
        ~AssetManager() noexcept = default;

        AssetManager(const AssetManager& that)            = delete;
        AssetManager(AssetManager&& that)                 = delete;
        AssetManager& operator=(const AssetManager& that) = delete;
        AssetManager& operator=(AssetManager&& that)      = delete;

        std::shared_ptr<Mesh>     loadMesh(const std::string& filePath);
        std::shared_ptr<Material> loadMaterial(const std::string& filePath);

    private:
        std::unordered_map<std::string, std::shared_ptr<Mesh>>     m_mesh_cache;
        std::unordered_map<std::string, std::shared_ptr<Material>> m_material_cache;
    };
} // namespace RealmEngine