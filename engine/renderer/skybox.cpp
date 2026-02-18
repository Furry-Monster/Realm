#include "renderer/skybox.h"

#include "core/geometry/primitive_vertices.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    Skybox::Skybox(RHIDevice& device, RHITexture* env_cubemap) : m_env_cubemap(env_cubemap)
    {
        m_cube = std::make_unique<IblCubeMesh>(createIblCubeMesh(device));
    }

    void Skybox::draw(RHIDevice& device) const
    {
        if (!m_env_cubemap || !m_cube || !m_cube->vertex_input)
            return;

        device.bindTexture(0, *m_env_cubemap);
        m_cube->vertex_input->draw(PrimitiveType::Triangles,
                                   static_cast<uint32_t>(PrimitiveVertices::k_cube_vertex_count));
    }

} // namespace RealmEngine
