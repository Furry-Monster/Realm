#include "renderer/ibl/hdri_cube.h"

#include "renderer/ibl/hdr_texture.h"
#include "renderer/ibl/ibl_geometry.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"
#include "rhi/rhi_vertex_input.h"

namespace RealmEngine
{
    HDRICube::HDRICube(RHIDevice& device, const std::string& hdri_path)
    {
        m_hdr_texture = std::make_unique<HDRTexture>(device, hdri_path);
        auto mesh     = createIblCubeMesh(device);
        m_cube        = std::make_unique<IblCubeMesh>(std::move(mesh));
    }

    void HDRICube::draw(RHIDevice& device, RHIShader& shader)
    {
        shader.setInt("hdri", 0);
        device.bindTexture(0, m_hdr_texture->getTexture());
        m_cube->vertex_input->draw(PrimitiveType::Triangles, 36);
    }
} // namespace RealmEngine
