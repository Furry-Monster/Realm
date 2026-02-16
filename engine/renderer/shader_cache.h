#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace RealmEngine
{
    class RHIDevice;
    class RHIShader;

    class ShaderCache
    {
    public:
        ShaderCache()           = default;
        ~ShaderCache() noexcept = default;

        ShaderCache(const ShaderCache&)            = delete;
        ShaderCache& operator=(const ShaderCache&) = delete;

        RHIShader* getOrCreate(const std::string& vert_path, const std::string& frag_path, RHIDevice& device);

        // Remove a specific cached shader (forces recompile on next getOrCreate)
        void invalidate(const std::string& vert_path, const std::string& frag_path);

        // Remove all cached shaders and failed records
        void clear();

    private:
        static std::string makeKey(const std::string& vert_path, const std::string& frag_path);

        std::unordered_map<std::string, std::unique_ptr<RHIShader>> m_cache;
        std::unordered_set<std::string>                             m_failed;
    };

} // namespace RealmEngine
