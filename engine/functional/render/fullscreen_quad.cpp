#include "functional/render/fullscreen_quad.h"

#include "core/math/primitive_vertices.h"
#include "functional/render/ibl/ibl_geometry.h"
#include "functional/render/rhi/rhi_device.h"

namespace RealmEngine
{
    struct FullscreenQuad::Impl
    {
        IblFullscreenQuadMesh mesh;
    };

    FullscreenQuad::FullscreenQuad(RHIDevice& device) : m_impl(std::make_unique<Impl>())
    {
        m_impl->mesh = createIblFullscreenQuadMesh(device);
    }

    FullscreenQuad::~FullscreenQuad() noexcept = default;

    void FullscreenQuad::draw() const
    {
        if (!m_impl || !m_impl->mesh.vertex_input)
            return;

        m_impl->mesh.vertex_input->draw(PrimitiveType::Triangles, PrimitiveVertices::k_fullscreen_quad_vertex_count);
    }

} // namespace RealmEngine
