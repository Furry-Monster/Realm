#include "renderer/fullscreen_quad.h"

#include "core/geometry/primitive_vertices.h"
#include "renderer/ibl/ibl_geometry.h"
#include "rhi/rhi_device.h"

namespace RealmEngine
{
    struct FullscreenQuad::Impl
    {
        IblFullscreenQuadMesh mesh;
    };

    FullscreenQuad::FullscreenQuad(RHIDevice& device)
    {
        m_impl       = new Impl;
        m_impl->mesh = createIblFullscreenQuadMesh(device);
    }

    FullscreenQuad::~FullscreenQuad()
    {
        delete m_impl;
        m_impl = nullptr;
    }

    void FullscreenQuad::draw() const
    {
        if (!m_impl || !m_impl->mesh.vertex_input)
            return;

        m_impl->mesh.vertex_input->draw(PrimitiveType::Triangles,
                                        static_cast<uint32_t>(PrimitiveVertices::k_fullscreen_quad_vertex_count));
    }

} // namespace RealmEngine
