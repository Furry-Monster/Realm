#pragma once

#include "render/ibl/brdf_convolution_framebuffer.h"
#include "render/ibl/mipmap_cubemap_framebuffer.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class Shader;

    /**
     * Computes specular maps to be used for PBR.
     *
     * The pre-filtered environment map has different mip levels for different roughness.
     *
     * The BRDF convolution map is a 2D map of NdotV vs. roughness, that provides F0 scale and F0 bias values.
     */
    class SpecularMap
    {
    public:
        /**
         * Initialize a specular map.
         */
        SpecularMap(const std::string& engineRoot, const unsigned int environmentCubemapId);

        /**
         * Render the pre-filtered environment map.
         */
        void computePrefilteredEnvMap();

        /**
         * Get the GL texture ID of the computed pre-filtered environment cubemap.
         * @return
         */
        unsigned int getPrefilteredEnvMapId() const;

        /**
         * Render the BRDF convolution map.
         */
        void computeBrdfConvolutionMap();

        /**
         * Get the GL texture ID of the computed BRDF convolution map.
         * @return
         */
        unsigned int getBrdfConvolutionMapId() const;

    private:
        // prefiltered environment map
        const unsigned int prefilteredEnvMapMipLevels = 5;
        const unsigned int prefilteredEnvMapWidth     = 128;
        const unsigned int prefilteredEnvMapHeight    = 128;

        const unsigned int environmentCubemapId;

        std::unique_ptr<Shader>                  prefilteredEnvMapShader;
        std::unique_ptr<MipmapCubemapFramebuffer> prefilteredEnvMapFramebuffer;

        // brdf convolution
        unsigned int brdfConvolutionMapId;
        const unsigned int brdfConvolutionMapWidth  = 512;
        const unsigned int brdfConvolutionMapHeight = 512;

        std::unique_ptr<Shader>                    brdfConvolutionShader;
        std::unique_ptr<BrdfConvolutionFramebuffer> brdfConvolutionFramebuffer;
    };
} // namespace RealmEngine

