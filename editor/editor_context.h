#pragma once

#include <entt/entity/entity.hpp>
#include <memory>
#include <string>

#include "hotkey/hotkey_manager.h"
#include "preferences/editor_preferences.h"
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
        EditorContext(EditorContext&&)                 = delete;
        EditorContext& operator=(EditorContext&&)      = delete;

        HotkeyManager&       getHotkeyManager() { return m_hotkeys; }
        const HotkeyManager& getHotkeyManager() const { return m_hotkeys; }

        void         setSelectedEntity(entt::entity entity) { m_selected_entity = entity; }
        entt::entity getSelectedEntity() const { return m_selected_entity; }
        bool         hasSelectedEntity() const { return m_selected_entity != entt::null; }
        void         clearSelectedEntity() { m_selected_entity = entt::null; }

        void                       setSelectedNode(std::shared_ptr<SceneNode> node) { m_selected_node = node; }
        std::shared_ptr<SceneNode> getSelectedNode() const { return m_selected_node; }
        bool                       hasSelectedNode() const { return m_selected_node != nullptr; }
        void                       clearSelectedNode() { m_selected_node.reset(); }

        /// Atomically select a node and its associated entity
        void selectNodeAndEntity(std::shared_ptr<SceneNode> node)
        {
            m_selected_node   = node;
            m_selected_entity = (node && node->hasEntity()) ? node->getEntity() : entt::null;
        }

        /// Atomically clear both selection states
        void clearSelection()
        {
            m_selected_node.reset();
            m_selected_entity = entt::null;
        }

        void        setEntityClipboard(const std::string& json) { m_entity_clipboard = json; }
        std::string getEntityClipboard() const { return m_entity_clipboard; }
        bool        hasEntityClipboard() const { return !m_entity_clipboard.empty(); }
        void        clearEntityClipboard() { m_entity_clipboard.clear(); }

        EditorPreferences&       getPreferences() { return m_preferences; }
        const EditorPreferences& getPreferences() const { return m_preferences; }

    private:
        HotkeyManager              m_hotkeys;
        entt::entity               m_selected_entity {entt::null};
        std::shared_ptr<SceneNode> m_selected_node {nullptr};
        std::string                m_entity_clipboard;
        EditorPreferences          m_preferences;
    };

} // namespace RealmEngine
