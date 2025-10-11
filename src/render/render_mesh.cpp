#include "render_mesh.h"

namespace RealmEngine
{
    VAOHandle    RenderMesh::getVAO() const { return m_vao; }
    BufferHandle RenderMesh::getVBO() const { return m_vbo; }
    BufferHandle RenderMesh::getEBO() const { return m_ebo; }
    void         RenderMesh::setVAO(VAOHandle vao) { m_vao = vao; }
    void         RenderMesh::setVBO(BufferHandle vbo) { m_vbo = vbo; }
    void         RenderMesh::setEBO(BufferHandle ebo) { m_ebo = ebo; }

} // namespace RealmEngine