#include "render/render_mesh.h"

#include "render/rhi/rhi_buffer.h"
#include "render/rhi/rhi_device.h"
#include "render/rhi/rhi_texture.h"
#include "render/rhi/rhi_vertex_input.h"

namespace RealmEngine
{
    static VertexLayout buildVertexLayout()
    {
        VertexLayout layout;
        layout.stride     = sizeof(RenderVertex);
        layout.attributes = {
            {0, 3, AttributeType::Float, offsetof(RenderVertex, m_position)},
            {1, 3, AttributeType::Float, offsetof(RenderVertex, m_normal)},
            {2, 2, AttributeType::Float, offsetof(RenderVertex, m_texture_coordinates)},
            {3, 3, AttributeType::Float, offsetof(RenderVertex, m_tangent)},
            {4, 3, AttributeType::Float, offsetof(RenderVertex, m_bitangent)},
        };
        return layout;
    }

    RenderMesh::RenderMesh(std::vector<RenderVertex> vertices,
                           std::vector<unsigned int> indices,
                           Material                  material,
                           RHIDevice&                device,
                           const std::string&        name) :
        m_name(name), m_vertices(std::move(vertices)), m_indices(std::move(indices)), m_material(std::move(material))
    {
        m_vertex_buffer = device.createBuffer(
            BufferType::Vertex, BufferUsage::Static, m_vertices.data(), m_vertices.size() * sizeof(RenderVertex));

        m_index_buffer = device.createBuffer(
            BufferType::Index, BufferUsage::Static, m_indices.data(), m_indices.size() * sizeof(unsigned int));

        m_vertex_input = device.createVertexInput(buildVertexLayout(), *m_vertex_buffer, m_index_buffer.get());
    }

    RenderMesh::~RenderMesh() noexcept = default;

    RenderMesh::RenderMesh(RenderMesh&& other) noexcept :
        m_name(std::move(other.m_name)), m_vertices(std::move(other.m_vertices)), m_indices(std::move(other.m_indices)),
        m_material(std::move(other.m_material)), m_vertex_buffer(std::move(other.m_vertex_buffer)),
        m_index_buffer(std::move(other.m_index_buffer)), m_vertex_input(std::move(other.m_vertex_input))
    {}

    RenderMesh& RenderMesh::operator=(RenderMesh&& other) noexcept
    {
        if (this != &other)
        {
            m_name          = std::move(other.m_name);
            m_vertices      = std::move(other.m_vertices);
            m_indices       = std::move(other.m_indices);
            m_material      = std::move(other.m_material);
            m_vertex_buffer = std::move(other.m_vertex_buffer);
            m_index_buffer  = std::move(other.m_index_buffer);
            m_vertex_input  = std::move(other.m_vertex_input);
        }
        return *this;
    }

    void RenderMesh::draw(RHIShader& shader)
    {
        m_material.properties.applyToShader(shader);
        m_vertex_input->drawIndexed(PrimitiveType::Triangles, static_cast<uint32_t>(m_indices.size()));
    }

    void RenderMesh::drawShadow([[maybe_unused]] RHIShader& shader)
    {
        m_vertex_input->drawIndexed(PrimitiveType::Triangles, static_cast<uint32_t>(m_indices.size()));
    }

    AABB RenderMesh::getLocalAABB() const
    {
        AABB box;
        for (const auto& v : m_vertices)
            box.merge(v.m_position);
        return box;
    }

} // namespace RealmEngine
