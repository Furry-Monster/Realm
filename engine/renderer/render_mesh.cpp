#include "renderer/render_mesh.h"

#include <glad/gl.h>

namespace RealmEngine
{
    RenderMesh::RenderMesh(std::vector<RenderVertex> vertices,
                           std::vector<unsigned int> indices,
                           RenderMaterial material) : m_vertices(vertices), m_indices(indices), m_material(material)
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices.size() * sizeof(RenderVertex),
                     &m_vertices[0],
                     GL_STATIC_DRAW); // copy over the vertex data

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_indices.size() * sizeof(unsigned int),
                     &m_indices[0],
                     GL_STATIC_DRAW); // copy over the index data

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), reinterpret_cast<void*>(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), reinterpret_cast<void*>(offsetof(RenderVertex, m_normal)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(RenderVertex),
                              reinterpret_cast<void*>(offsetof(RenderVertex, m_texture_coordinates)));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), reinterpret_cast<void*>(offsetof(RenderVertex, m_tangent)));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(RenderVertex),
                              reinterpret_cast<void*>(offsetof(RenderVertex, m_bitangent)));

        glBindVertexArray(0);
    }

    RenderMesh::~RenderMesh()
    {
        if (m_vao != 0)
            glDeleteVertexArrays(1, &m_vao);
        if (m_vbo != 0)
            glDeleteBuffers(1, &m_vbo);
        if (m_ebo != 0)
            glDeleteBuffers(1, &m_ebo);
    }

    RenderMesh::RenderMesh(RenderMesh&& other) noexcept :
        m_vertices(std::move(other.m_vertices)), m_indices(std::move(other.m_indices)),
        m_material(std::move(other.m_material)), m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo)
    {
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
    }

    RenderMesh& RenderMesh::operator=(RenderMesh&& other) noexcept
    {
        if (this != &other)
        {
            if (m_vao != 0)
                glDeleteVertexArrays(1, &m_vao);
            if (m_vbo != 0)
                glDeleteBuffers(1, &m_vbo);
            if (m_ebo != 0)
                glDeleteBuffers(1, &m_ebo);

            m_vertices = std::move(other.m_vertices);
            m_indices  = std::move(other.m_indices);
            m_material = std::move(other.m_material);
            m_vao      = other.m_vao;
            m_vbo      = other.m_vbo;
            m_ebo      = other.m_ebo;

            other.m_vao = 0;
            other.m_vbo = 0;
            other.m_ebo = 0;
        }
        return *this;
    }

    void RenderMesh::draw(Shader& shader)
    {
        // Material uniforms + texture binding via Texture::bind() to avoid scattered raw GL calls.
        // TODO: Migrate fully to RHI once Texture system is refactored.

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

        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
} // namespace RealmEngine
