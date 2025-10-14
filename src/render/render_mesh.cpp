#include "render_mesh.h"
#include "resource/datatype/model/mesh.h"
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <glad/gl.h>

namespace RealmEngine
{
    VAOHandle    RenderMesh::getVAO() const { return m_vao; }
    BufferHandle RenderMesh::getVBO() const { return m_vbo; }
    BufferHandle RenderMesh::getEBO() const { return m_ebo; }
    uint32_t     RenderMesh::getIndexCount() const { return m_index_count; }
    uint32_t     RenderMesh::getVertexCount() const { return m_vertex_count; }

    void RenderMesh::sync(RHI& rhi, const Mesh& mesh)
    {
        if (!mesh.isGpuDataDirty())
            return;

        const auto& verts   = mesh.getVertices();
        const auto& indices = mesh.getIndices();

        if (verts.empty() || indices.empty())
        {
            warn("Trying to sync an empty Mesh to RenderMesh.");
            return;
        }

        if (m_vao == 0)
            m_vao = rhi.createVAO();

        bind();

        if (m_vbo == 0)
            m_vbo = rhi.createBuffer(GL_ARRAY_BUFFER, sizeof(Vertex) * verts.size(), verts.data(), GL_STATIC_DRAW);
        else
            rhi.updateBuffer(m_vbo, GL_ARRAY_BUFFER, 0, sizeof(Vertex) * verts.size(), verts.data());

        if (m_ebo == 0)
            m_ebo = rhi.createBuffer(
                GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
        else
            rhi.updateBuffer(m_ebo, GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * indices.size(), indices.data());

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // position (location=0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));

        // normal (location=1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

        // uv0 (location=2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex_coord)));

        // tangent (location=3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tangent)));

        // bitangent (location=4)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(
            4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, bitangent)));

        // vert color (location=5)
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(
            5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

        unbind();

        m_index_count  = static_cast<uint32_t>(indices.size());
        m_vertex_count = static_cast<uint32_t>(verts.size());

        const_cast<Mesh&>(mesh).markGpuDataSynced(); // NOTE:bad code...
    }
    void RenderMesh::disposal(RHI& rhi)
    {
        if (m_vao != 0)
        {
            rhi.deleteVAO(m_vao);
            m_vao = 0;
        }

        if (m_vbo != 0)
        {
            rhi.deleteBuffer(m_vbo);
            m_vbo = 0;
        }

        if (m_ebo != 0)
        {
            rhi.deleteBuffer(m_ebo);
            m_ebo = 0;
        }

        m_vertex_count = 0;
        m_index_count  = 0;
    }

    void RenderMesh::bind() const { glBindVertexArray(m_vao); }
    void RenderMesh::unbind() const { glBindVertexArray(0); }

} // namespace RealmEngine