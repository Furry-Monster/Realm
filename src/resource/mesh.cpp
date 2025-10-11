#include "mesh.h"
#include <utility>

namespace RealmEngine
{
    Mesh::Mesh(Mesh&& that) noexcept :
        m_verts(std::move(that.m_verts)), m_indices(std::move(that.m_indices)),
        m_submeshes(std::move(that.m_submeshes)), m_aabb(std::move(that.m_aabb)), m_gpu_data_dirty(true)
    {}

    Mesh& Mesh::operator=(Mesh&& that) noexcept
    {
        if (this != &that)
        {
            m_verts          = std::move(that.m_verts);
            m_indices        = std::move(that.m_indices);
            m_submeshes      = std::move(that.m_submeshes);
            m_aabb           = std::move(that.m_aabb);
            m_gpu_data_dirty = true;
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

    void Mesh::setVertices(std::vector<Vertex>&& vertices)
    {
        m_verts          = std::move(vertices);
        m_gpu_data_dirty = true;
    }

    void Mesh::setIndices(std::vector<uint32_t>&& indices)
    {
        m_indices        = std::move(indices);
        m_gpu_data_dirty = true;
    }

    void Mesh::addSubMesh(const SubMesh& submesh) { m_submeshes.push_back(submesh); }

    void Mesh::calculateNormals() {}

    void Mesh::calculateTangents() {}

    void Mesh::calculateBounds() {}

    bool Mesh::isValid() const { return true; }

} // namespace RealmEngine