#include "render_mesh.h"
#include <glad/gl.h>

namespace RealmEngine
{
    VAOHandle    RenderMesh::getVAO() const { return m_vao; }
    BufferHandle RenderMesh::getVBO() const { return m_vbo; }
    BufferHandle RenderMesh::getEBO() const { return m_ebo; }
    uint32_t     RenderMesh::getIndexCount() const { return m_index_count; }
    uint32_t     RenderMesh::getVertexCount() const { return m_vertex_count; }

    void RenderMesh::setVAO(VAOHandle vao) { m_vao = vao; }
    void RenderMesh::setVBO(BufferHandle vbo) { m_vbo = vbo; }
    void RenderMesh::setEBO(BufferHandle ebo) { m_ebo = ebo; }
    void RenderMesh::setIndexCount(uint32_t count) { m_index_count = count; }
    void RenderMesh::setVertexCount(uint32_t count) { m_vertex_count = count; }

    void RenderMesh::bind() const { glBindVertexArray(m_vao); }
    void RenderMesh::unbind() const { glBindVertexArray(0); }

} // namespace RealmEngine