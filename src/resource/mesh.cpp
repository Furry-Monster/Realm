#include "mesh.h"
#include "glm/geometric.hpp"
#include <cmath>
#include <utility>

namespace RealmEngine
{
    const std::vector<Vertex>&   Mesh::getVertices() const { return m_verts; }
    const std::vector<uint32_t>& Mesh::getIndices() const { return m_indices; }
    const std::vector<SubMesh>&  Mesh::getSubMeshes() const { return m_submeshes; }
    const AABB&                  Mesh::getBounds() const { return m_aabb; }

    std::vector<Vertex>&   Mesh::getVertices() { return m_verts; }
    std::vector<uint32_t>& Mesh::getIndices() { return m_indices; }
    std::vector<SubMesh>&  Mesh::getSubMeshes() { return m_submeshes; }
    AABB&                  Mesh::getAABB() { return m_aabb; }

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

    void           Mesh::addSubMesh(const SubMesh& submesh) { m_submeshes.push_back(submesh); }
    void           Mesh::clearSubMeshes() { m_submeshes.clear(); }
    const SubMesh* Mesh::findSubMesh(const std::string& name) const
    {
        for (const auto& submesh : m_submeshes)
        {
            if (submesh.name == name)
                return &submesh;
        }
        return nullptr;
    }

    bool Mesh::isGpuDataDirty() const { return m_gpu_data_dirty; }
    void Mesh::markGpuDataSynced() { m_gpu_data_dirty = false; }

    void Mesh::calculateNormals()
    {
        if (m_indices.empty() || m_verts.empty())
            return;

        for (auto& vert : m_verts)
            vert.normal = glm::vec3(0.0f);

        for (size_t i = 0; i < m_indices.size(); i += 3)
        {
            uint32_t idx0 = m_indices[i];
            uint32_t idx1 = m_indices[i + 1];
            uint32_t idx2 = m_indices[i + 2];

            if (idx0 >= m_verts.size() || idx1 >= m_verts.size() || idx2 >= m_verts.size())
                continue;

            const glm::vec3& p0 = m_verts[idx0].position;
            const glm::vec3& p1 = m_verts[idx1].position;
            const glm::vec3& p2 = m_verts[idx2].position;

            glm::vec3 edge1       = p1 - p0;
            glm::vec3 edge2       = p2 - p0;
            glm::vec3 face_normal = glm::cross(edge1, edge2);

            m_verts[idx0].normal += face_normal;
            m_verts[idx1].normal += face_normal;
            m_verts[idx2].normal += face_normal;
        }

        for (auto& vert : m_verts)
            if (glm::length(vert.normal) > 0.0f)
                vert.normal = glm::normalize(vert.normal);

        m_gpu_data_dirty = true;
    }

    void Mesh::calculateTangents()
    {
        if (m_indices.empty() || m_verts.empty())
            return;

        for (auto& vert : m_verts)
        {
            vert.tangent   = glm::vec3(0.0f);
            vert.bitangent = glm::vec3(0.0f);
        }

        for (size_t i = 0; i < m_indices.size(); i += 3)
        {
            uint32_t idx0 = m_indices[i];
            uint32_t idx1 = m_indices[i + 1];
            uint32_t idx2 = m_indices[i + 2];

            if (idx0 >= m_verts.size() || idx1 >= m_verts.size() || idx2 >= m_verts.size())
                continue;

            const glm::vec3& p0 = m_verts[idx0].position;
            const glm::vec3& p1 = m_verts[idx1].position;
            const glm::vec3& p2 = m_verts[idx2].position;

            const glm::vec2& uv0 = m_verts[idx0].tex_coord;
            const glm::vec2& uv1 = m_verts[idx1].tex_coord;
            const glm::vec2& uv2 = m_verts[idx2].tex_coord;

            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;

            glm::vec2 delta_uv1 = uv1 - uv0;
            glm::vec2 delta_uv2 = uv2 - uv0;

            float det = delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y;

            constexpr float epsilon = 1e-6f;
            if (std::abs(det) < epsilon)
                continue;

            float f = 1.0f / det;

            glm::vec3 tangent;
            tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
            tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
            tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);

            glm::vec3 bitangent;
            bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
            bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
            bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);

            m_verts[idx0].tangent += tangent;
            m_verts[idx1].tangent += tangent;
            m_verts[idx2].tangent += tangent;

            m_verts[idx0].bitangent += bitangent;
            m_verts[idx1].bitangent += bitangent;
            m_verts[idx2].bitangent += bitangent;
        }

        for (auto& vert : m_verts)
        {
            if (glm::length(vert.tangent) > 0.0f)
                // Gram-Schmidt orthogonalization
                vert.tangent = glm::normalize(vert.tangent - vert.normal * glm::dot(vert.normal, vert.tangent));

            if (glm::length(vert.bitangent) > 0.0f)
                vert.bitangent = glm::normalize(vert.bitangent);
        }

        m_gpu_data_dirty = true;
    }

    void Mesh::calculateAABB()
    {
        if (m_verts.empty())
        {
            m_aabb.min = glm::vec3(0.0f);
            m_aabb.max = glm::vec3(0.0f);
            return;
        }

        m_aabb.min = m_verts[0].position;
        m_aabb.max = m_verts[0].position;

        for (const auto& vert : m_verts)
        {
            m_aabb.min = glm::min(m_aabb.min, vert.position);
            m_aabb.max = glm::max(m_aabb.max, vert.position);
        }
    }

    bool Mesh::isValid() const
    {
        if (m_verts.empty() || m_indices.empty())
            return false;

        for (uint32_t idx : m_indices)
            if (idx >= m_verts.size())
                return false;

        if (m_indices.size() % 3 != 0)
            return false;

        for (const auto& submesh : m_submeshes)
        {
            if (!submesh.isValid())
                return false;

            if (submesh.base_index >= m_indices.size())
                return false;

            if (submesh.getEndIndex() > m_indices.size())
                return false;
        }

        return true;
    }

    void Mesh::clear()
    {
        m_verts.clear();
        m_indices.clear();
        m_submeshes.clear();
        m_aabb.min       = glm::vec3(0.0f);
        m_aabb.max       = glm::vec3(0.0f);
        m_gpu_data_dirty = true;
    }

} // namespace RealmEngine