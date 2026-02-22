#pragma once

#include <memory>
#include <string>

#include "functional/render/ibl/hdr_texture.h"
#include "functional/render/ibl/ibl_geometry.h"

namespace RealmEngine
{
    class RHIDevice;
    class RHIShader;

    class HDRICube
    {
    public:
        HDRICube(RHIDevice& device, const std::string& hdri_path);
        void draw(RHIDevice& device, RHIShader& shader);

    private:
        std::unique_ptr<HDRTexture>  m_hdr_texture;
        std::unique_ptr<IblCubeMesh> m_cube;
    };

} // namespace RealmEngine
