#pragma once

#include <entt/entity/entity.hpp>
#include <memory>
#include <vector>

#include "widget.h"

namespace RealmEngine
{
    class EditorContext;
    class EventBus;
    class SceneManager;

    class EntityBrowserWidget : public Widget
    {
    public:
        EntityBrowserWidget(std::shared_ptr<EditorContext> context, SceneManager& scene_mgr, EventBus& event_bus);
        ~EntityBrowserWidget() override = default;

        EntityBrowserWidget(const EntityBrowserWidget&)            = delete;
        EntityBrowserWidget& operator=(const EntityBrowserWidget&) = delete;
        EntityBrowserWidget(EntityBrowserWidget&&)                 = delete;
        EntityBrowserWidget& operator=(EntityBrowserWidget&&)      = delete;

        void render() override;

    private:
        void        collectEntitiesWithComponent();
        std::string getEntityDisplayName(entt::entity entity) const;

        std::shared_ptr<EditorContext> m_context;
        SceneManager&                  m_scene_mgr;
        EventBus&                      m_event_bus;

        int                       m_selected_component_type {0};
        std::vector<entt::entity> m_filtered_entities;
    };

} // namespace RealmEngine
