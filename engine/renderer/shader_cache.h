#pragma once

#include <memory>
#include <string>
#include <unordered_map>

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
        void       clear();

    private:
        static std::string makeKey(const std::string& vert_path, const std::string& frag_path);

        std::unordered_map<std::string, std::unique_ptr<RHIShader>> m_cache;
    };

} // namespace RealmEngine
