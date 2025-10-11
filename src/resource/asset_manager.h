#pragma once

#include "utils.h"

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
    };
} // namespace RealmEngine