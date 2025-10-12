#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

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
        size_t                                    getChilrenCount() const;
        void                                      addChild(std::unique_ptr<Node> child);

        // Mesh & material management
        std::optional<uint32_t> getMeshIndex() const;
        std::optional<uint32_t> getMaterialIndex() const;
        void                    setMeshIndex(uint32_t mesh_idx);
        void                    setMaterialIndex(uint32_t material_idx);

        // Transform
        const glm::mat4& getLocalTransform() const;
        void             setLocalTransform(const glm::mat4& transform);
        glm::mat4        getWorldTransform() const;

    private:
        glm::mat4               m_local_trans {1.0f};
        std::optional<uint32_t> m_mesh_idx;
        std::optional<uint32_t> m_material_idx;

        Node*                              m_parent;
        std::vector<std::unique_ptr<Node>> m_children;
    };
} // namespace RealmEngine