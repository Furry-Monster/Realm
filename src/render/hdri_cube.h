#pragma once

#include <memory>
#include <string>
#include "render/cube.h"
#include "render/hdr_texture.h"

namespace RealmEngine
{
    class Shader;

    class HDRICube
    {
    public:
        explicit HDRICube(const std::string& hdri_path);
        void draw(Shader& shader);

    private:
        std::unique_ptr<Cube> m_cube;
        HDRTexture            m_hdr_texture;
    };
} // namespace RealmEngine
