#pragma once

#include <memory>
#include <string>
#include "core/math/cube.h"
#include "renderer/ibl/hdr_texture.h"

namespace RealmEngine
{
    class RHIShader;

    class HDRICube
    {
    public:
        explicit HDRICube(const std::string& hdri_path);
        void draw(RHIShader& shader);

    private:
        std::unique_ptr<Cube> m_cube;
        HDRTexture            m_hdr_texture;
    };
} // namespace RealmEngine
