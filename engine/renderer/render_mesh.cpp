#include "renderer/render_mesh.h"

#include "rhi/rhi_buffer.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_vertex_input.h"

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
                           RenderMaterial            material,
                           RHIDevice&                device) :
        m_vertices(std::move(vertices)), m_indices(std::move(indices)), m_material(std::move(material))
    {
        m_vertex_buffer = device.createBuffer(
            BufferType::Vertex, BufferUsage::Static, m_vertices.data(), m_vertices.size() * sizeof(RenderVertex));

        m_index_buffer = device.createBuffer(
            BufferType::Index, BufferUsage::Static, m_indices.data(), m_indices.size() * sizeof(unsigned int));

        m_vertex_input = device.createVertexInput(buildVertexLayout(), *m_vertex_buffer, m_index_buffer.get());
    }

    RenderMesh::~RenderMesh() = default;

    RenderMesh::RenderMesh(RenderMesh&& other) noexcept :
        m_vertices(std::move(other.m_vertices)), m_indices(std::move(other.m_indices)),
        m_material(std::move(other.m_material)), m_vertex_buffer(std::move(other.m_vertex_buffer)),
        m_index_buffer(std::move(other.m_index_buffer)), m_vertex_input(std::move(other.m_vertex_input))
    {}

    RenderMesh& RenderMesh::operator=(RenderMesh&& other) noexcept
    {
        if (this != &other)
        {
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
        // Bind PBR material textures
        shader.setBool("material.useTextureAlbedo", m_material.use_texture_albedo);
        shader.setVec3("material.albedo", m_material.albedo);
        if (m_material.use_texture_albedo && m_material.texture_albedo)
        {
            m_material.texture_albedo->bind(TEXTURE_UNIT_ALBEDO);
            shader.setInt("material.textureAlbedo", TEXTURE_UNIT_ALBEDO);
        }

        shader.setBool("material.useTextureMetallicRoughness", m_material.use_texture_metallic_roughness);
        shader.setFloat("material.metallic", m_material.metallic);
        shader.setFloat("material.roughness", m_material.roughness);
        if (m_material.use_texture_metallic_roughness && m_material.texture_metallic_roughness)
        {
            m_material.texture_metallic_roughness->bind(TEXTURE_UNIT_METALLIC_ROUGHNESS);
            shader.setInt("material.textureMetallicRoughness", TEXTURE_UNIT_METALLIC_ROUGHNESS);
        }

        shader.setBool("material.useTextureNormal", m_material.use_texture_normal);
        if (m_material.use_texture_normal && m_material.texture_normal)
        {
            m_material.texture_normal->bind(TEXTURE_UNIT_NORMAL);
            shader.setInt("material.textureNormal", TEXTURE_UNIT_NORMAL);
        }

        shader.setBool("material.useTextureAmbientOcclusion", m_material.use_texture_ambient_occlusion);
        shader.setFloat("material.ambientOcclusion", m_material.ambient_occlusion);
        if (m_material.use_texture_ambient_occlusion && m_material.texture_ambient_occlusion)
        {
            m_material.texture_ambient_occlusion->bind(TEXTURE_UNIT_AMBIENT_OCCLUSION);
            shader.setInt("material.textureAmbientOcclusion", TEXTURE_UNIT_AMBIENT_OCCLUSION);
        }

        shader.setBool("material.useTextureEmissive", m_material.use_texture_emissive);
        shader.setVec3("material.emissive", m_material.emissive);
        if (m_material.use_texture_emissive && m_material.texture_emissive)
        {
            m_material.texture_emissive->bind(TEXTURE_UNIT_EMISSIVE);
            shader.setInt("material.textureEmissive", TEXTURE_UNIT_EMISSIVE);
        }

        m_vertex_input->drawIndexed(PrimitiveType::Triangles, static_cast<uint32_t>(m_indices.size()));
    }
} // namespace RealmEngine
