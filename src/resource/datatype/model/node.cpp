#include "node.h"
#include <memory>

namespace RealmEngine
{
    // Hierarchy management
    const Node*                               Node::getParent() const { return m_parent; }
    const std::vector<std::unique_ptr<Node>>& Node::getChildren() const { return m_children; }
    size_t                                    Node::getChildrenCount() const { return m_children.size(); }
    void                                      Node::addChild(std::unique_ptr<Node> child)
    {
        if (child)
        {
            child->m_parent = this;
            m_children.push_back(std::move(child));
        }
    }

    // Mesh management
    const std::vector<uint32_t>& Node::getMeshIndices() const { return m_mesh_indices; }
    void                         Node::addMeshIndex(uint32_t mesh_idx) { m_mesh_indices.push_back(mesh_idx); }
    void Node::setMeshIndices(const std::vector<uint32_t>& indices) { m_mesh_indices = indices; }
    bool Node::hasMeshes() const { return !m_mesh_indices.empty(); }

    // Transform
    const glm::mat4& Node::getLocalTransform() const { return m_local_trans; }
    void             Node::setLocalTransform(const glm::mat4& transform) { m_local_trans = transform; }
    glm::mat4        Node::getWorldTransform() const
    {
        if (m_parent)
            return m_parent->getWorldTransform() * m_local_trans;

        return m_local_trans;
    }
} // namespace RealmEngine