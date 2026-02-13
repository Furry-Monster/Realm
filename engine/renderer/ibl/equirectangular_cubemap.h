#pragma once

#include <memory>
#include <string>
#include "renderer/ibl/cubemap_framebuffer.h"
#include "renderer/ibl/hdri_cube.h"

namespace RealmEngine
{
    class GLShader;

    class EquirectangularCubemap
    {
    public:
        EquirectangularCubemap(const std::string& engineRoot, const std::string& hdriPath);
        ~EquirectangularCubemap();

        void compute();

        unsigned int getCubemapId() const;

    private:
        const unsigned int m_cubemap_width  = 512;
        const unsigned int m_cubemap_height = 512;

        std::unique_ptr<GLShader>           m_hdri_shader;
        std::unique_ptr<HDRICube>           m_hdri_cube;
        std::unique_ptr<CubemapFramebuffer> m_framebuffer;
    };
} // namespace RealmEngine
