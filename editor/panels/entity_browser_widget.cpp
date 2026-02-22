#include "panels/entity_browser_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "editor_context.h"
#include "functional/ecs/components/name_tag.h"
#include "module/camera/components/camera.h"
#include "module/render/components/lighting/area.h"
#include "module/render/components/lighting/directional.h"
#include "module/render/components/lighting/point.h"
#include "module/render/components/lighting/spot.h"
#include "module/render/components/renderable.h"
#include "functional/ecs/components/transform.h"
#include "functional/scene/scene.h"

#include <imgui.h>
#include <entt/entity/registry.hpp>

namespace RealmEngine
{
    EntityBrowserWidget::EntityBrowserWidget(const std::shared_ptr<EditorContext>& context,
                                             EditorEngineBridge&                   bridge) :
        Widget("Entity Browser"), m_context(context), m_bridge(&bridge)
    {}

    void EntityBrowserWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_name.c_str(), &m_open))
        {
            ImGui::End();
            return;
        }

        const auto scene = m_bridge->getCurrentScene();
        if (!scene)
        {
            ImGui::Text("No scene loaded");
            ImGui::End();
            return;
        }

        const char* component_types[] = {
            "Transform", "Renderable", "Camera", "PointLight", "SpotLight", "DirectionalLight", "AreaLight", "NameTag"};
        constexpr int num_types = 8;

        if (ImGui::Combo("Filter by component", &m_selected_component_type, component_types, num_types))
        {
        }
        collectEntitiesWithComponent();

        ImGui::Separator();
        ImGui::Text("Entities: %zu", m_filtered_entities.size());
        ImGui::BeginChild("EntityList", ImVec2(0, 0), true);

        for (const entt::entity entity : m_filtered_entities)
        {
            if (!scene->valid(entity))
                continue;

            std::string label = getEntityDisplayName(entity);
            const bool  selected =
                m_context && m_context->hasSelectedEntity() && m_context->getSelectedEntity() == entity;

            if (ImGui::Selectable(label.c_str(), selected))
            {
                if (m_context)
                {
                    m_context->setSelectedEntity(entity);
                    auto node = scene->findNodeByEntity(entity);
                    m_context->setSelectedNode(node);
                    m_bridge->getEventBus().publish(EntitySelectedEvent {entity, node ? node.get() : nullptr});
                }
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void EntityBrowserWidget::collectEntitiesWithComponent()
    {
        m_filtered_entities.clear();
        const auto scene = m_bridge->getCurrentScene();
        if (!scene)
            return;

        entt::registry& reg = scene->getRegistry();

        switch (m_selected_component_type)
        {
            case 0:
                for (auto entity : reg.view<Transform>())
                    m_filtered_entities.push_back(entity);
                break;
            case 1:
                for (auto entity : reg.view<Renderable>())
                    m_filtered_entities.push_back(entity);
                break;
            case 2:
                for (auto entity : reg.view<Camera>())
                    m_filtered_entities.push_back(entity);
                break;
            case 3:
                for (auto entity : reg.view<PointLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 4:
                for (auto entity : reg.view<SpotLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 5:
                for (auto entity : reg.view<DirectionalLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 6:
                for (auto entity : reg.view<AreaLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 7:
                for (auto entity : reg.view<NameTag>())
                    m_filtered_entities.push_back(entity);
                break;
            default:
                break;
        }
    }

    std::string EntityBrowserWidget::getEntityDisplayName(entt::entity entity) const
    {
        const auto scene = m_bridge->getCurrentScene();
        if (!scene || !scene->valid(entity))
            return "Invalid";

        if (auto* tag = scene->tryGet<NameTag>(entity))
            return tag->name;
        return "Entity " + std::to_string(static_cast<uint32_t>(entity));
    }

} // namespace RealmEngine
