#include "renderer/shader_cache.h"

#include "core/log/log_macros.h"
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
        auto        it  = m_cache.find(key);
        if (it != m_cache.end())
            return it->second.get();

        auto shader = device.createShader(vert_path, frag_path);
        if (!shader || !shader->isValid())
        {
            RE_LOG_ERROR("ShaderCache: failed to compile custom shader [" + vert_path + ", " + frag_path + "]");
            return nullptr;
        }

        RHIShader* raw = shader.get();
        m_cache.emplace(key, std::move(shader));
        return raw;
    }

    void ShaderCache::clear() { m_cache.clear(); }

} // namespace RealmEngine
