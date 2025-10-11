#include "mesh.h"
#include "render/resource_manager.h"
#include <algorithm>
#include <utility>

namespace RealmEngine
{
    Mesh::Mesh(Mesh&& that) noexcept :
        m_verts(std::move(that.m_verts)), m_indices(std::move(that.m_indices)),
        m_submeshes(std::move(that.m_submeshes)), m_aabb(std::move(that.m_aabb)), m_vao(that.m_vao), m_vbo(that.m_vbo),
        m_ebo(that.m_ebo), m_gpu_data_dirty(that.m_gpu_data_dirty)
    {
        that.m_vao            = 0;
        that.m_vbo            = 0;
        that.m_ebo            = 0;
        that.m_gpu_data_dirty = true;
    }

    Mesh& Mesh::operator=(Mesh&& that) noexcept
    {
        if (this != &that)
        {
            m_verts     = std::move(that.m_verts);
            m_indices   = std::move(that.m_indices);
            m_submeshes = std::move(that.m_submeshes);
            m_aabb      = std::move(that.m_aabb);

            m_vao            = that.m_vao;
            m_vbo            = that.m_vbo;
            m_ebo            = that.m_ebo;
            m_gpu_data_dirty = that.m_gpu_data_dirty;

            that.m_vao            = 0;
            that.m_vbo            = 0;
            that.m_ebo            = 0;
            that.m_gpu_data_dirty = true;
        }

        return *this;
    }

    const std::vector<Vertex>&   Mesh::getVertices() const { return m_verts; }
    const std::vector<uint32_t>& Mesh::getIndices() const { return m_indices; }
    const std::vector<SubMesh>&  Mesh::getSubMeshes() const { return m_submeshes; }
    const AABB&                  Mesh::getBounds() const { return m_aabb; }

    std::vector<Vertex>&   Mesh::getVertices() { return m_verts; }
    std::vector<uint32_t>& Mesh::getIndices() { return m_indices; }
    std::vector<SubMesh>&  Mesh::getSubMeshes() { return m_submeshes; }

    void Mesh::setVertices(std::vector<Vertex>&& vertices) {}
    void Mesh::setIndices(std::vector<uint32_t>&& indices) {}
    void Mesh::addSubMesh(const SubMesh& submesh) {}

    void Mesh::calculateNormals() {}

    void Mesh::calculateTangents() {}

    void Mesh::calculateBounds() {}

    bool Mesh::isValid() const { return true; }

    VAOHandle Mesh::getVAO() const { return m_vao; }

    BufferHandle Mesh::getVBO() const { return m_vbo; }

    BufferHandle Mesh::getEBO() const { return m_ebo; }

    void Mesh::setVAO(VAOHandle vao) { m_vao = vao; }

    void Mesh::setVBO(BufferHandle vbo) { m_vbo = vbo; }

    void Mesh::setEBO(BufferHandle ebo) { m_ebo = ebo; }

} // namespace RealmEngine