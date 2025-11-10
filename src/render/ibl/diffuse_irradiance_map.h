#pragma once

#include "render/ibl/cubemap_framebuffer.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class Shader;

    /**
     * Computes a diffuse irradiance map from an environment map.
     *
     * The computed map tells you the sum of incoming light from the environment
     * for a particular direction.
     */
    class DiffuseIrradianceMap
    {
    public:
        /**
         * Initialize a diffuse irradiance map.
         * @param engineRoot root path of the engine
         * @param environmentCubemapId GL texture ID of the environment cubemap
         */
        DiffuseIrradianceMap(const std::string& engineRoot, const unsigned int environmentCubemapId);

        /**
         * Render the diffuse irradiance map.
         */
        void compute();

        /**
         * Get the GL texture ID of the computed cubemap.
         * @return
         */
        unsigned int getCubemapId() const;

    private:
        const unsigned int m_diffuse_irradiance_map_width  = 32;
        const unsigned int m_diffuse_irradiance_map_height = 32;

        const unsigned int m_environment_cubemap_id;

        std::unique_ptr<Shader>              m_diffuse_irradiance_shader;
        std::unique_ptr<CubemapFramebuffer>  m_diffuse_irradiance_framebuffer;
    };
} // namespace RealmEngine

