#pragma once

#include <memory>
#include <string>

namespace RealmEngine
{
    class RHIDevice;
    class RHIFramebuffer;
    class RHIShader;
    class RHITexture;

    class SpecularMap
    {
    public:
        SpecularMap(RHIDevice& device, const std::string& engine_root, RHITexture* environment_cubemap);
        void computePrefilteredEnvMap(RHIDevice& device);
        void computeBrdfConvolutionMap(RHIDevice& device);

        RHITexture* getPrefilteredEnvMapTexture() const;
        RHITexture* getBrdfConvolutionTexture() const;

    private:
        static constexpr int m_prefilter_mip_levels = 5;
        static constexpr int m_prefilter_width      = 128;
        static constexpr int m_prefilter_height     = 128;
        static constexpr int m_brdf_width           = 512;
        static constexpr int m_brdf_height          = 512;

        RHITexture*                     m_environment_cubemap;
        std::unique_ptr<RHIShader>      m_prefilter_shader;
        std::unique_ptr<RHIFramebuffer> m_prefilter_framebuffer;
        std::unique_ptr<RHIShader>      m_brdf_shader;
        std::unique_ptr<RHIFramebuffer> m_brdf_framebuffer;
    };

} // namespace RealmEngine
