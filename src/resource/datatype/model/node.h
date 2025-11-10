#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "glm/ext/matrix_float4x4.hpp"

namespace RealmEngine
{
    class Node
    {
    public:
        Node()           = default;
        ~Node() noexcept = default;

        Node(const Node&)                = delete;
        Node& operator=(const Node&)     = delete;
        Node(Node&&) noexcept            = default;
        Node& operator=(Node&&) noexcept = default;

        // Hierarchy management
        const Node*                               getParent() const;
        const std::vector<std::unique_ptr<Node>>& getChildren() const;
        size_t                                    getChildrenCount() const;
        void                                      addChild(std::unique_ptr<Node> child);

        // Mesh management
        const std::vector<uint32_t>& getMeshIndices() const;
        void                         addMeshIndex(uint32_t mesh_idx);
        void                         setMeshIndices(const std::vector<uint32_t>& indices);
        bool                         hasMeshes() const;

        // Transform
        const glm::mat4& getLocalTransform() const;
        void             setLocalTransform(const glm::mat4& transform);
        glm::mat4        getWorldTransform() const;

    private:
        glm::mat4                          m_local_trans {1.0f};
        std::vector<uint32_t>              m_mesh_indices;
        Node*                              m_parent {nullptr};
        std::vector<std::unique_ptr<Node>> m_children;
    };
} // namespace RealmEngine