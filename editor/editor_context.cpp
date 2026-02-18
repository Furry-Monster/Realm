#include "editor_context.h"

namespace RealmEngine
{

    /**
     * @brief
     * Atomically select a node and its associated entity
     */
    void EditorContext::selectNodeAndEntity(const std::shared_ptr<SceneNode>& node)
    {
        m_selected_node   = node;
        m_selected_entity = (node && node->hasEntity()) ? node->getEntity() : entt::null;
    }

    /**
     * @brief
     * Atomically select a node and its associated entity
     */
    void EditorContext::clearSelection()
    {
        m_selected_node.reset();
        m_selected_entity = entt::null;
    }
} // namespace RealmEngine