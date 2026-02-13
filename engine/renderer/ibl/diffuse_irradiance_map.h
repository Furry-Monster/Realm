#pragma once

#include <memory>
#include <string>

namespace RealmEngine
{
    class RHIDevice;
    class RHIFramebuffer;
    class RHIShader;
    class RHITexture;

    class DiffuseIrradianceMap
    {
    public:
        DiffuseIrradianceMap(RHIDevice& device, const std::string& engine_root, RHITexture* environment_cubemap);
        void compute(RHIDevice& device);

        RHITexture* getCubemapTexture() const;

    private:
        static constexpr int m_width  = 32;
        static constexpr int m_height = 32;

        RHITexture*                     m_environment_cubemap;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
    };

} // namespace RealmEngine
