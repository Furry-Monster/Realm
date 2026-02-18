#include "render/ibl/hdri_cube.h"

#include "core/geometry/primitive_vertices.h"

#include "render/ibl/hdr_texture.h"
#include "render/ibl/ibl_geometry.h"
#include "render/rhi/rhi_device.h"
#include "render/rhi/rhi_shader.h"
#include "render/rhi/rhi_texture.h"
#include "render/rhi/rhi_types.h"
#include "render/rhi/rhi_vertex_input.h"

namespace RealmEngine
{
    HDRICube::HDRICube(RHIDevice& device, const std::string& hdri_path)
    {
        m_hdr_texture = std::make_unique<HDRTexture>(device, hdri_path);
        if (!m_hdr_texture->isValid())
        {
            m_hdr_texture.reset();
            return;
        }
        auto mesh = createIblCubeMesh(device);
        m_cube    = std::make_unique<IblCubeMesh>(std::move(mesh));
    }

    void HDRICube::draw(RHIDevice& device, RHIShader& shader)
    {
        if (!m_hdr_texture || !m_cube)
            return;
        shader.setInt("hdri", 0);
        device.bindTexture(0, m_hdr_texture->getTexture());
        m_cube->vertex_input->draw(PrimitiveType::Triangles,
                                   static_cast<uint32_t>(PrimitiveVertices::k_cube_vertex_count));
    }
} // namespace RealmEngine
