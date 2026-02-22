#include "functional/resource/asset_manager.h"

#include <stb_image.h>

#include <algorithm>
#include <cmath>

#include "core/base/macros.h"
#include "functional/render/render_object.h"
#include "functional/render/rhi/rhi_device.h"
#include "functional/render/rhi/rhi_texture.h"
#include "functional/render/rhi/rhi_types.h"
#include "functional/resource/model_loader.h"

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

        auto meshes = ModelLoader::load(path, flip_textures, device, this);
        if (meshes.empty())
            return nullptr;

        auto                        obj = std::make_shared<RenderObject>(std::move(meshes));
        std::lock_guard<std::mutex> lock(m_model_mutex);
        auto [it, inserted] = m_model_cache.emplace(key, obj);
        return it->second;
    }

    std::shared_ptr<RHITexture> AssetManager::getOrLoadTexture(const std::string& path,
                                                               const std::string& directory,
                                                               bool               is_srgb,
                                                               RHIDevice&         device)
    {
        std::string full_path = path;
        if (!path.empty() && path[0] != '/' && (path.length() <= 1 || path[1] != ':'))
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
        auto [it, inserted] = m_texture_cache.emplace(key, shared_texture);
        return it->second;
    }

    std::shared_ptr<RHITexture> AssetManager::getOrLoadTextureForPreview(const std::string& full_path,
                                                                         RHIDevice&         device)
    {
        std::string ext;
        if (full_path.size() >= 4)
        {
            ext = full_path.substr(full_path.size() - 4);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }

        if (ext == ".hdr")
        {
            std::string key = "preview_hdr:" + full_path;
            {
                std::lock_guard<std::mutex> lock(m_texture_mutex);
                auto                        it = m_texture_cache.find(key);
                if (it != m_texture_cache.end())
                    return it->second;
            }

            int    width, height, num_channels;
            float* data = stbi_loadf(full_path.c_str(), &width, &height, &num_channels, 4);
            if (!data)
            {
                RE_LOG_ERROR("Failed to load HDR texture for preview: " + full_path);
                return nullptr;
            }

            const size_t               ldr_data_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
            std::vector<unsigned char> ldr_data(ldr_data_size);
            for (size_t i = 0; i < ldr_data_size; ++i)
            {
                float v     = data[i];
                v           = v / (1.0f + v);
                ldr_data[i] = static_cast<unsigned char>(std::clamp(v * 255.0f, 0.0f, 255.0f));
            }
            stbi_image_free(data);

            TextureDesc desc;
            desc.type       = TextureType::Texture2D;
            desc.format     = TextureFormat::RGBA8;
            desc.width      = width;
            desc.height     = height;
            desc.min_filter = TextureFilter::Linear;
            desc.mag_filter = TextureFilter::Linear;
            desc.wrap_s     = TextureWrap::ClampToEdge;
            desc.wrap_t     = TextureWrap::ClampToEdge;
            desc.data       = ldr_data.data();

            auto texture = device.createTexture(desc);
            if (!texture)
                return nullptr;

            std::shared_ptr<RHITexture> shared_texture(std::move(texture));
            std::lock_guard<std::mutex> lock(m_texture_mutex);
            m_texture_cache[key] = shared_texture;
            return shared_texture;
        }

        return getOrLoadTexture(full_path, "", true, device);
    }

} // namespace RealmEngine
