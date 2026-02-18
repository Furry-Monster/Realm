#pragma once

#include <entt/entity/entity.hpp>
#include <memory>
#include <string>

#include "hotkey/hotkey_manager.h"
#include "preferences/editor_preferences.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    enum class GizmoOperation
    {
        None,
        Translate,
        Rotate,
        Scale
    };

    class EditorContext
    {
    public:
        EditorContext()  = default;
        ~EditorContext() noexcept = default;

        EditorContext(const EditorContext&)            = delete;
        EditorContext& operator=(const EditorContext&) = delete;
        EditorContext(EditorContext&&)                 = delete;
        EditorContext& operator=(EditorContext&&)      = delete;

        HotkeyManager&       getHotkeyManager() { return m_hotkeys; }
        const HotkeyManager& getHotkeyManager() const { return m_hotkeys; }

        void         setSelectedEntity(const entt::entity entity) { m_selected_entity = entity; }
        entt::entity getSelectedEntity() const { return m_selected_entity; }
        bool         hasSelectedEntity() const { return m_selected_entity != entt::null; }
        void         clearSelectedEntity() { m_selected_entity = entt::null; }

        void                       setSelectedNode(const std::shared_ptr<SceneNode>& node) { m_selected_node = node; }
        std::shared_ptr<SceneNode> getSelectedNode() const { return m_selected_node; }
        bool                       hasSelectedNode() const { return m_selected_node != nullptr; }
        void                       clearSelectedNode() { m_selected_node.reset(); }

        void selectNodeAndEntity(const std::shared_ptr<SceneNode>& node);
        void clearSelection();

        void        setEntityClipboard(const std::string& json) { m_entity_clipboard = json; }
        std::string getEntityClipboard() const { return m_entity_clipboard; }
        bool        hasEntityClipboard() const { return !m_entity_clipboard.empty(); }
        void        clearEntityClipboard() { m_entity_clipboard.clear(); }

        EditorPreferences&       getPreferences() { return m_preferences; }
        const EditorPreferences& getPreferences() const { return m_preferences; }

        void           setGizmoOperation(const GizmoOperation op) { m_gizmo_operation = op; }
        GizmoOperation getGizmoOperation() const { return m_gizmo_operation; }

    private:
        HotkeyManager              m_hotkeys;
        entt::entity               m_selected_entity {entt::null};
        std::shared_ptr<SceneNode> m_selected_node {nullptr};
        std::string                m_entity_clipboard;
        EditorPreferences          m_preferences;
        GizmoOperation             m_gizmo_operation {GizmoOperation::Translate};
    };

} // namespace RealmEngine
