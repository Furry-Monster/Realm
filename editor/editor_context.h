#pragma once

#include <entt/entity/entity.hpp>
#include <memory>
#include "scene/scene_node.h"

namespace RealmEngine
{
    class EditorContext
    {
    public:
        EditorContext()  = default;
        ~EditorContext() = default;

        EditorContext(const EditorContext&)            = delete;
        EditorContext& operator=(const EditorContext&) = delete;
        EditorContext(EditorContext&&)                 = default;
        EditorContext& operator=(EditorContext&&)      = default;

        void         setSelectedEntity(entt::entity entity) { m_selected_entity = entity; }
        entt::entity getSelectedEntity() const { return m_selected_entity; }
        bool         hasSelectedEntity() const { return m_selected_entity != entt::null; }
        void         clearSelectedEntity() { m_selected_entity = entt::null; }

        void                       setSelectedNode(std::shared_ptr<SceneNode> node) { m_selected_node = node; }
        std::shared_ptr<SceneNode> getSelectedNode() const { return m_selected_node; }
        bool                       hasSelectedNode() const { return m_selected_node != nullptr; }
        void                       clearSelectedNode() { m_selected_node.reset(); }

    private:
        entt::entity               m_selected_entity {entt::null};
        std::shared_ptr<SceneNode> m_selected_node {nullptr};
    };

} // namespace RealmEngine
