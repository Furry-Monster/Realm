#pragma once

#include <memory>
#include <string>

namespace RealmEngine
{
    class RHIDevice;
    class RHITexture;

    // Loads HDR equirectangular texture via RHI (no direct OpenGL).
    class HDRTexture
    {
    public:
        HDRTexture(RHIDevice& device, const std::string& path);

        bool              isValid() const { return m_texture != nullptr; }
        RHITexture&       getTexture() { return *m_texture; }
        const RHITexture& getTexture() const { return *m_texture; }

    private:
        std::unique_ptr<RHITexture> m_texture;
    };
} // namespace RealmEngine
