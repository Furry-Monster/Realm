#pragma once

#include "render/ibl/cubemap_framebuffer.h"
#include "render/hdri_cube.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class Shader;

    /**
     * A cubemap texture created from an equirectangular image.
     */
    class EquirectangularCubemap
    {
    public:
        /**
         * Initialize an EquirectangularCubemap.
         * @param engineRoot root path of the engine
         * @param hdriPath path to an .hdr image in equirectangular projection format
         */
        EquirectangularCubemap(const std::string& engineRoot, const std::string& hdriPath);

        /**
         * Render the equirectangular cubemap.
         */
        void compute();

        /**
         * Get the GL texture ID of the computed cubemap.
         * @return
         */
        unsigned int getCubemapId() const;

    private:
        const unsigned int cubemapWidth  = 512;
        const unsigned int cubemapHeight = 512;

        std::unique_ptr<Shader>              hdriShader;
        std::unique_ptr<HDRICube>           hdriCube;
        std::unique_ptr<CubemapFramebuffer> framebuffer;
    };
} // namespace RealmEngine

