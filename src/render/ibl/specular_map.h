#pragma once

#include <memory>
#include <string>
#include "render/ibl/brdf_convolution_framebuffer.h"
#include "render/ibl/mipmap_cubemap_framebuffer.h"

namespace RealmEngine
{
    class Shader;

    class SpecularMap
    {
    public:
        SpecularMap(const std::string& engineRoot, unsigned int environmentCubemapId);

        void         computePrefilteredEnvMap();
        unsigned int getPrefilteredEnvMapId() const;

        void         computeBrdfConvolutionMap();
        unsigned int getBrdfConvolutionMapId() const;

    private:
        // prefiltered environment map
        const unsigned int m_prefiltered_env_map_mip_levels = 5;
        const unsigned int m_prefiltered_env_map_width      = 128;
        const unsigned int m_prefiltered_env_map_height     = 128;

        const unsigned int m_environment_cubemap_id;

        std::unique_ptr<Shader>                   m_prefiltered_env_map_shader;
        std::unique_ptr<MipmapCubemapFramebuffer> m_prefiltered_env_map_framebuffer;

        // brdf convolution
        unsigned int       m_brdf_convolution_map_id;
        const unsigned int m_brdf_convolution_map_width  = 512;
        const unsigned int m_brdf_convolution_map_height = 512;

        std::unique_ptr<Shader>                     m_brdf_convolution_shader;
        std::unique_ptr<BrdfConvolutionFramebuffer> m_brdf_convolution_framebuffer;
    };
} // namespace RealmEngine
