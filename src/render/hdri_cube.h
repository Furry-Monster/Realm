#pragma once

#include "render/cube.h"
#include "render/hdr_texture.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class Shader;

    /**
     * A unit cube model textured with an equirectangular HDR image.
     */
    class HDRICube
    {
    public:
        explicit HDRICube(const std::string& hdriPath);
        void draw(Shader& shader);

    private:
        std::unique_ptr<Cube> mCube;
        HDRTexture             mHdrTexture;
    };
} // namespace RealmEngine

