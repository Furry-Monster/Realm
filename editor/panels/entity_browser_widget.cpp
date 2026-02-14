#include "panels/entity_browser_widget.h"

#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "editor_context.h"
#include "scene/components/lighting/area.h"
#include "scene/components/lighting/directional.h"
#include "scene/components/lighting/point.h"
#include "scene/components/lighting/spot.h"
#include "scene/components/name_tag.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"

#include <imgui.h>
#include <entt/entity/registry.hpp>

namespace RealmEngine
{
    EntityBrowserWidget::EntityBrowserWidget(std::shared_ptr<EditorContext> context,
                                             SceneManager&                  scene_mgr,
                                             EventBus&                      event_bus) :
        Widget("Entity Browser"), m_context(context), m_scene_mgr(scene_mgr), m_event_bus(event_bus)
    {}

    void EntityBrowserWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_name.c_str(), &m_open))
        {
            ImGui::End();
            return;
        }

        auto scene = m_scene_mgr.getCurrentScene();
        if (!scene)
        {
            ImGui::Text("No scene loaded");
            ImGui::End();
            return;
        }

        const char* component_types[] = {
            "Transform", "Renderable", "PointLight", "SpotLight", "DirectionalLight", "AreaLight", "NameTag"};
        const int num_types = 7;

        if (ImGui::Combo("Filter by component", &m_selected_component_type, component_types, num_types))
        {
        }
        collectEntitiesWithComponent();

        ImGui::Separator();
        ImGui::Text("Entities: %zu", m_filtered_entities.size());
        ImGui::BeginChild("EntityList", ImVec2(0, 0), true);

        for (entt::entity entity : m_filtered_entities)
        {
            if (!scene->valid(entity))
                continue;

            std::string label = getEntityDisplayName(entity);
            bool selected     = m_context && m_context->hasSelectedEntity() && m_context->getSelectedEntity() == entity;

            if (ImGui::Selectable(label.c_str(), selected))
            {
                if (m_context)
                {
                    m_context->setSelectedEntity(entity);
                    auto node = scene->findNodeByEntity(entity);
                    m_context->setSelectedNode(node);
                    m_event_bus.publish(EntitySelectedEvent {entity, node ? node.get() : nullptr});
                }
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void EntityBrowserWidget::collectEntitiesWithComponent()
    {
        m_filtered_entities.clear();
        auto scene = m_scene_mgr.getCurrentScene();
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
                for (auto entity : reg.view<PointLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 3:
                for (auto entity : reg.view<SpotLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 4:
                for (auto entity : reg.view<DirectionalLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 5:
                for (auto entity : reg.view<AreaLight>())
                    m_filtered_entities.push_back(entity);
                break;
            case 6:
                for (auto entity : reg.view<NameTag>())
                    m_filtered_entities.push_back(entity);
                break;
            default:
                break;
        }
    }

    std::string EntityBrowserWidget::getEntityDisplayName(entt::entity entity) const
    {
        auto scene = m_scene_mgr.getCurrentScene();
        if (!scene || !scene->valid(entity))
            return "Invalid";

        if (auto* tag = scene->tryGet<NameTag>(entity))
            return tag->name;
        return "Entity " + std::to_string(static_cast<uint32_t>(entity));
    }

} // namespace RealmEngine
