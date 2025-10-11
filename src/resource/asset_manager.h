#pragma once

#include <filesystem>
#include <string>

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

        template<typename AssetType>
        bool loadAsset(const std::string& asset_url, AssetType& asset) const
        {}

        template<typename AssetType>
        bool saveAsset(AssetType& asset, const std::string& asset_url) const
        {}

        std::filesystem::path getFullPath(const std::string& relative_path) const;
    };
} // namespace RealmEngine