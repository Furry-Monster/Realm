#include "editor_context.h"

namespace RealmEngine
{
    void EditorContext::setSelectedEntity(std::shared_ptr<Entity> entity) { m_selected_entity = entity; }

    std::shared_ptr<Entity> EditorContext::getSelectedEntity() const { return m_selected_entity; }

    bool EditorContext::hasSelectedEntity() const { return m_selected_entity != nullptr; }

    void EditorContext::clearSelectedEntity() { m_selected_entity.reset(); }

    void EditorContext::setSelectedNode(std::shared_ptr<SceneNode> node) { m_selected_node = node; }

    std::shared_ptr<SceneNode> EditorContext::getSelectedNode() const { return m_selected_node; }

    bool EditorContext::hasSelectedNode() const { return m_selected_node != nullptr; }

    void EditorContext::clearSelectedNode() { m_selected_node.reset(); }

} // namespace RealmEngine
