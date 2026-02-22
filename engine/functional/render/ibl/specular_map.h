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
        static constexpr int PREFILTER_MIP_LEVELS = 5;
        static constexpr int PREFILTER_WIDTH      = 128;
        static constexpr int PREFILTER_HEIGHT     = 128;
        static constexpr int BRDF_WIDTH           = 512;
        static constexpr int BRDF_HEIGHT          = 512;

        RHITexture*                     m_environment_cubemap;
        std::unique_ptr<RHIShader>      m_prefilter_shader;
        std::unique_ptr<RHIFramebuffer> m_prefilter_framebuffer;
        std::unique_ptr<RHIShader>      m_brdf_shader;
        std::unique_ptr<RHIFramebuffer> m_brdf_framebuffer;
    };

} // namespace RealmEngine
