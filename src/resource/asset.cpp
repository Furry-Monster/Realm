#include "asset.h"
#include "glm/common.hpp"
#include <utility>

namespace RealmEngine
{
    // node tree
    Node*       Asset::getRoot() { return m_root.get(); }
    const Node* Asset::getRoot() const { return m_root.get(); }
    void        Asset::setRoot(std::unique_ptr<Node> root) { m_root = std::move(root); }

    // mesh management
    size_t      Asset::getMeshCount() const { return m_meshes.size(); }
    Mesh&       Asset::getMesh(size_t idx) { return m_meshes[idx]; }
    const Mesh& Asset::getMesh(size_t idx) const { return m_meshes[idx]; }
    const Mesh* Asset::tryGetMesh(size_t idx) const
    {
        if (idx >= m_meshes.size())
            return nullptr;
        return &m_meshes[idx];
    }
    size_t Asset::addMesh(Mesh&& mesh)
    {
        m_meshes.push_back(std::move(mesh));
        return m_meshes.size() - 1;
    }
    void Asset::clearMeshes() { m_meshes.clear(); }

    // material management
    size_t          Asset::getMaterialCount() const { return m_materials.size(); }
    Material&       Asset::getMaterial(size_t idx) { return m_materials[idx]; }
    const Material& Asset::getMaterial(size_t idx) const { return m_materials[idx]; }
    const Material* Asset::tryGetMaterial(size_t idx) const
    {
        if (idx >= m_materials.size())
            return nullptr;
        return &m_materials[idx];
    }
    size_t Asset::addMaterial(Material&& material)
    {
        m_materials.push_back(std::move(material));
        return m_materials.size() - 1;
    }
    void Asset::clearMaterials() { m_materials.clear(); }

    // misc
    void Asset::clear()
    {
        m_root.reset();
        m_meshes.clear();
        m_materials.clear();
    }
    bool Asset::isEmpty() const { return m_root == nullptr && m_meshes.empty() && m_materials.empty(); }
    AABB Asset::calculateAABB() const
    {
        AABB result;
        if (m_meshes.empty())
        {
            result.min = glm::vec3(0.0f);
            result.max = glm::vec3(0.0f);
            return result;
        }

        bool first = true;
        for (const auto& mesh : m_meshes)
        {
            const AABB& mesh_aabb = mesh.getBounds();
            if (first)
            {
                result.min = mesh_aabb.min;
                result.max = mesh_aabb.max;
                first      = false;
            }
            else
            {
                result.min = glm::min(result.min, mesh_aabb.min);
                result.max = glm::max(result.max, mesh_aabb.max);
            }
        }

        return result;
    }

} // namespace RealmEngine