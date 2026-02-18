#pragma once

#include <entt/entity/entity.hpp>
#include <memory>
#include <vector>

#include "widget.h"

namespace RealmEngine
{
    class EditorContext;
    class EditorEngineBridge;

    class EntityBrowserWidget : public Widget
    {
    public:
        EntityBrowserWidget(const std::shared_ptr<EditorContext>& context, EditorEngineBridge& bridge);
        ~EntityBrowserWidget() noexcept override = default;

        EntityBrowserWidget(const EntityBrowserWidget&)            = delete;
        EntityBrowserWidget& operator=(const EntityBrowserWidget&) = delete;
        EntityBrowserWidget(EntityBrowserWidget&&)                 = delete;
        EntityBrowserWidget& operator=(EntityBrowserWidget&&)      = delete;

        void render() override;

    private:
        void        collectEntitiesWithComponent();
        std::string getEntityDisplayName(entt::entity entity) const;

        std::shared_ptr<EditorContext> m_context;
        EditorEngineBridge*            m_bridge;

        int                       m_selected_component_type {0};
        std::vector<entt::entity> m_filtered_entities;
    };

} // namespace RealmEngine
