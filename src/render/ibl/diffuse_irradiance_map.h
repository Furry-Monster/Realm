#pragma once

#include <memory>
#include <string>
#include "render/ibl/cubemap_framebuffer.h"

namespace RealmEngine
{
    class Shader;

    class DiffuseIrradianceMap
    {
    public:
        DiffuseIrradianceMap(const std::string& engineRoot, unsigned int environmentCubemapId);

        void compute();

        unsigned int getCubemapId() const;

    private:
        const unsigned int m_diffuse_irradiance_map_width  = 32;
        const unsigned int m_diffuse_irradiance_map_height = 32;

        const unsigned int m_environment_cubemap_id;

        std::unique_ptr<Shader>             m_diffuse_irradiance_shader;
        std::unique_ptr<CubemapFramebuffer> m_diffuse_irradiance_framebuffer;
    };
} // namespace RealmEngine
