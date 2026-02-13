#pragma once

#include <memory>
#include <string>

#include "renderer/ibl/hdri_cube.h"

namespace RealmEngine
{
    class RHIDevice;
    class RHIFramebuffer;
    class RHIShader;
    class RHITexture;

    class EquirectangularCubemap
    {
    public:
        EquirectangularCubemap(RHIDevice& device, const std::string& engine_root, const std::string& hdri_path);
        void compute(RHIDevice& device);

        RHITexture* getCubemapTexture() const;

    private:
        static constexpr int m_cubemap_width  = 512;
        static constexpr int m_cubemap_height = 512;

        std::unique_ptr<RHIShader>      m_hdri_shader;
        std::unique_ptr<HDRICube>       m_hdri_cube;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
    };

} // namespace RealmEngine
