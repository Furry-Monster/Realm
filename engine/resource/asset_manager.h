#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace RealmEngine
{
    class RHIDevice;
    class RHITexture;
    class RenderObject;

    class AssetManager
    {
    public:
        AssetManager()           = default;
        ~AssetManager() noexcept = default;

        AssetManager(const AssetManager&)                = delete;
        AssetManager& operator=(const AssetManager&)     = delete;
        AssetManager(AssetManager&&) noexcept            = delete;
        AssetManager& operator=(AssetManager&&) noexcept = delete;

        void initialize();
        void disposal();

        std::shared_ptr<RenderObject> getOrLoadModel(const std::string& path, bool flip_textures, RHIDevice& device);

        std::shared_ptr<RHITexture>
        getOrLoadTexture(const std::string& path, const std::string& directory, bool is_srgb, RHIDevice& device);

        std::shared_ptr<RHITexture> getOrLoadTextureForPreview(const std::string& full_path, RHIDevice& device);

    private:
        std::string makeTextureCacheKey(const std::string& path, bool is_srgb) const;

        std::unordered_map<std::string, std::shared_ptr<RenderObject>> m_model_cache;
        std::unordered_map<std::string, std::shared_ptr<RHITexture>>   m_texture_cache;
        std::mutex                                                     m_model_mutex;
        std::mutex                                                     m_texture_mutex;
    };

} // namespace RealmEngine
