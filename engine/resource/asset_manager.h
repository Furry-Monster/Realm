#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace RealmEngine
{
    class RenderObject;

    class AssetManager
    {
    public:
        AssetManager()           = default;
        ~AssetManager() noexcept = default;

        AssetManager(const AssetManager&)                = delete;
        AssetManager& operator=(const AssetManager&)     = delete;
        AssetManager(AssetManager&&) noexcept            = default;
        AssetManager& operator=(AssetManager&&) noexcept = default;

        void initialize();
        void disposal();

    private:
        // Future: cache loaded models, textures, shaders by path
    };
} // namespace RealmEngine
