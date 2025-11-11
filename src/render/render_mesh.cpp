#include "render/render_mesh.h"

#include <glad/gl.h>

namespace RealmEngine
{
    RenderMesh::RenderMesh(std::vector<RenderVertex> vertices,
                           std::vector<unsigned int> indices,
                           RenderMaterial material) : m_vertices(vertices), m_indices(indices), m_material(material)
    {
        init();
    }

    void RenderMesh::draw(Shader& shader)
    {
        shader.setBool("material.useTextureAlbedo", m_material.use_texture_albedo);
        shader.setVec3("material.albedo", m_material.albedo);
        if (m_material.use_texture_albedo)
        {
            glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_ALBEDO);
            shader.setInt("material.textureAlbedo", TEXTURE_UNIT_ALBEDO);
            glBindTexture(GL_TEXTURE_2D, m_material.texture_albedo->m_id);
        }

        shader.setBool("material.useTextureMetallicRoughness", m_material.use_texture_metallic_roughness);
        shader.setFloat("material.metallic", m_material.metallic);
        shader.setFloat("material.roughness", m_material.roughness);
        if (m_material.use_texture_metallic_roughness)
        {
            glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_METALLIC_ROUGHNESS);
            shader.setInt("material.textureMetallicRoughness", TEXTURE_UNIT_METALLIC_ROUGHNESS);
            glBindTexture(GL_TEXTURE_2D, m_material.texture_metallic_roughness->m_id);
        }

        shader.setBool("material.useTextureNormal", m_material.use_texture_normal);
        if (m_material.use_texture_normal)
        {
            glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_NORMAL);
            shader.setInt("material.textureNormal", TEXTURE_UNIT_NORMAL);
            glBindTexture(GL_TEXTURE_2D, m_material.texture_normal->m_id);
        }

        shader.setBool("material.useTextureAmbientOcclusion", m_material.use_texture_ambient_occlusion);
        shader.setFloat("material.ambientOcclusion", m_material.ambient_occlusion);
        if (m_material.use_texture_ambient_occlusion)
        {
            glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_AMBIENT_OCCLUSION);
            shader.setInt("material.textureAmbientOcclusion", TEXTURE_UNIT_AMBIENT_OCCLUSION);
            glBindTexture(GL_TEXTURE_2D, m_material.texture_ambient_occlusion->m_id);
        }

        shader.setBool("material.useTextureEmissive", m_material.use_texture_emissive);
        shader.setVec3("material.emissive", m_material.emissive);
        if (m_material.use_texture_emissive)
        {
            glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_EMISSIVE);
            shader.setInt("material.textureEmissive", TEXTURE_UNIT_EMISSIVE);
            glBindTexture(GL_TEXTURE_2D, m_material.texture_emissive->m_id);
        }

        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void RenderMesh::init()
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
} // namespace RealmEngine
