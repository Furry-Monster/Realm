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

    RenderMesh::~RenderMesh() = default;

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
        const RenderMaterial& mat = m_material;

        shader.setBool("material.useTextureAlbedo", mat.use_texture_albedo);
        shader.setVec3("material.albedo", mat.albedo);
        if (mat.use_texture_albedo && mat.texture_albedo)
        {
            mat.texture_albedo->bind(TEXTURE_UNIT_ALBEDO);
            shader.setInt("material.textureAlbedo", TEXTURE_UNIT_ALBEDO);
        }

        shader.setBool("material.useTextureOpacity", mat.use_texture_opacity);
        shader.setFloat("material.opacity", mat.opacity);
        shader.setFloat("material.alphaCutout", mat.alpha_cutout);
        if (mat.use_texture_opacity && mat.texture_opacity)
        {
            mat.texture_opacity->bind(TEXTURE_UNIT_OPACITY);
            shader.setInt("material.textureOpacity", TEXTURE_UNIT_OPACITY);
        }

        shader.setBool("material.useTextureMetallicRoughness", mat.use_texture_metallic_roughness);
        shader.setFloat("material.metallic", mat.metallic);
        shader.setFloat("material.roughness", mat.roughness);
        if (mat.use_texture_metallic_roughness && mat.texture_metallic_roughness)
        {
            mat.texture_metallic_roughness->bind(TEXTURE_UNIT_METALLIC_ROUGHNESS);
            shader.setInt("material.textureMetallicRoughness", TEXTURE_UNIT_METALLIC_ROUGHNESS);
        }

        shader.setBool("material.useTextureNormal", mat.use_texture_normal);
        if (mat.use_texture_normal && mat.texture_normal)
        {
            mat.texture_normal->bind(TEXTURE_UNIT_NORMAL);
            shader.setInt("material.textureNormal", TEXTURE_UNIT_NORMAL);
        }

        shader.setBool("material.useTextureAmbientOcclusion", mat.use_texture_ambient_occlusion);
        shader.setFloat("material.ambientOcclusion", mat.ambient_occlusion);
        if (mat.use_texture_ambient_occlusion && mat.texture_ambient_occlusion)
        {
            mat.texture_ambient_occlusion->bind(TEXTURE_UNIT_AMBIENT_OCCLUSION);
            shader.setInt("material.textureAmbientOcclusion", TEXTURE_UNIT_AMBIENT_OCCLUSION);
        }

        shader.setBool("material.useTextureEmissive", mat.use_texture_emissive);
        shader.setVec3("material.emissive", mat.emissive);
        shader.setFloat("material.emissiveStrength", mat.emissive_strength);
        if (mat.use_texture_emissive && mat.texture_emissive)
        {
            mat.texture_emissive->bind(TEXTURE_UNIT_EMISSIVE);
            shader.setInt("material.textureEmissive", TEXTURE_UNIT_EMISSIVE);
        }

        shader.setBool("material.subsurfaceEnabled", mat.subsurface_enabled);
        shader.setFloat("material.subsurfaceRadius", mat.subsurface_radius);
        shader.setVec3("material.subsurfaceColor", mat.subsurface_color);

        m_vertex_input->drawIndexed(PrimitiveType::Triangles, static_cast<uint32_t>(m_indices.size()));
    }

    void RenderMesh::drawHair(RHIShader& shader)
    {
        if (!m_material.is_hair)
            return;

        const RenderMaterial& mat = m_material;

        shader.setBool("material.useTextureAlbedo", mat.use_texture_albedo);
        shader.setVec3("material.albedo", mat.albedo);
        if (mat.use_texture_albedo && mat.texture_albedo)
        {
            mat.texture_albedo->bind(TEXTURE_UNIT_ALBEDO);
            shader.setInt("material.textureAlbedo", TEXTURE_UNIT_ALBEDO);
        }

        shader.setBool("material.useTextureOpacity", mat.use_texture_opacity);
        shader.setFloat("material.opacity", mat.opacity);
        if (mat.use_texture_opacity && mat.texture_opacity)
        {
            mat.texture_opacity->bind(TEXTURE_UNIT_OPACITY);
            shader.setInt("material.textureOpacity", TEXTURE_UNIT_OPACITY);
        }

        shader.setBool("material.useTextureEmissive", mat.use_texture_emissive);
        shader.setVec3("material.emissive", mat.emissive);
        shader.setFloat("material.emissiveStrength", mat.emissive_strength);
        if (mat.use_texture_emissive && mat.texture_emissive)
        {
            mat.texture_emissive->bind(TEXTURE_UNIT_EMISSIVE);
            shader.setInt("material.textureEmissive", TEXTURE_UNIT_EMISSIVE);
        }

        shader.setFloat("material.specularStrength", mat.hair_specular_strength);
        shader.setFloat("material.specularPower", mat.hair_specular_power);

        m_vertex_input->drawIndexed(PrimitiveType::Triangles, static_cast<uint32_t>(m_indices.size()));
    }

    void RenderMesh::drawShadow([[maybe_unused]] RHIShader& shader)
    {
        m_vertex_input->drawIndexed(PrimitiveType::Triangles, static_cast<uint32_t>(m_indices.size()));
    }
} // namespace RealmEngine
