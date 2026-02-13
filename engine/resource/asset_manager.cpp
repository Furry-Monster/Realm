#include "resource/asset_manager.h"

#include <stb/stb_image.h>

#include "core/log/log_macros.h"
#include "renderer/render_object.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    void AssetManager::initialize() {}

    void AssetManager::disposal()
    {
        std::lock_guard<std::mutex> lock_model(m_model_mutex);
        std::lock_guard<std::mutex> lock_tex(m_texture_mutex);
        m_model_cache.clear();
        m_texture_cache.clear();
    }

    std::string AssetManager::makeTextureCacheKey(const std::string& path, bool is_srgb) const
    {
        return path + (is_srgb ? "_srgb" : "_lin");
    }

    std::shared_ptr<RenderObject>
    AssetManager::getOrLoadModel(const std::string& path, bool flip_textures, RHIDevice& device)
    {
        std::string key = path + (flip_textures ? "_flip" : "");
        {
            std::lock_guard<std::mutex> lock(m_model_mutex);
            auto                        it = m_model_cache.find(key);
            if (it != m_model_cache.end())
                return it->second;
        }

        auto obj = std::make_shared<RenderObject>(path, flip_textures, device, this);
        if (obj->isEmpty())
            return nullptr;

        std::lock_guard<std::mutex> lock(m_model_mutex);
        m_model_cache[key] = obj;
        return obj;
    }

    std::shared_ptr<RHITexture> AssetManager::getOrLoadTexture(const std::string& path,
                                                               const std::string& directory,
                                                               bool               is_srgb,
                                                               RHIDevice&         device)
    {
        std::string full_path = path;
        if (path[0] != '/' && (path.length() <= 1 || path[1] != ':'))
            full_path = directory + '/' + path;

        std::string key = makeTextureCacheKey(full_path, is_srgb);
        {
            std::lock_guard<std::mutex> lock(m_texture_mutex);
            auto                        it = m_texture_cache.find(key);
            if (it != m_texture_cache.end())
                return it->second;
        }

        int            width, height, num_channels;
        unsigned char* data = stbi_load(full_path.c_str(), &width, &height, &num_channels, 0);
        if (!data)
        {
            RE_LOG_ERROR("Failed to load texture: " + full_path);
            return nullptr;
        }

        TextureFormat format;
        switch (num_channels)
        {
            case 1:
                format = TextureFormat::R8;
                break;
            case 3:
                format = is_srgb ? TextureFormat::SRGB8 : TextureFormat::RGB8;
                break;
            case 4:
                format = is_srgb ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
                break;
            default:
                RE_LOG_ERROR("Unsupported texture channels: " + std::to_string(num_channels));
                stbi_image_free(data);
                return nullptr;
        }

        TextureDesc desc;
        desc.type       = TextureType::Texture2D;
        desc.format     = format;
        desc.width      = width;
        desc.height     = height;
        desc.min_filter = TextureFilter::Linear;
        desc.mag_filter = TextureFilter::Linear;
        desc.wrap_s     = TextureWrap::Repeat;
        desc.wrap_t     = TextureWrap::Repeat;
        desc.gen_mips   = true;
        desc.data       = data;

        auto texture = device.createTexture(desc);
        stbi_image_free(data);

        if (!texture)
            return nullptr;

        std::shared_ptr<RHITexture> shared_texture(std::move(texture));
        std::lock_guard<std::mutex> lock(m_texture_mutex);
        m_texture_cache[key] = shared_texture;
        return shared_texture;
    }

} // namespace RealmEngine
