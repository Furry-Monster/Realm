#include "renderer/shader_cache.h"

#include "core/base/macros.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    std::string ShaderCache::makeKey(const std::string& vert_path, const std::string& frag_path)
    {
        return vert_path + "|" + frag_path;
    }

    RHIShader* ShaderCache::getOrCreate(const std::string& vert_path, const std::string& frag_path, RHIDevice& device)
    {
        if (vert_path.empty() || frag_path.empty())
            return nullptr;

        std::string key = makeKey(vert_path, frag_path);

        if (m_failed.count(key))
            return nullptr;

        const auto it = m_cache.find(key);
        if (it != m_cache.end())
            return it->second.get();

        auto shader = device.createShader(vert_path, frag_path);
        if (!shader || !shader->isValid())
        {
            RE_LOG_ERROR("ShaderCache: failed to compile custom shader [" + vert_path + ", " + frag_path + "]");
            m_failed.insert(key);
            return nullptr;
        }

        RHIShader* raw = shader.get();
        m_cache.emplace(key, std::move(shader));
        return raw;
    }

    void ShaderCache::invalidate(const std::string& vert_path, const std::string& frag_path)
    {
        const std::string key = makeKey(vert_path, frag_path);
        m_cache.erase(key);
        m_failed.erase(key);
    }

    void ShaderCache::clear()
    {
        m_cache.clear();
        m_failed.clear();
    }

} // namespace RealmEngine
