#include "panels/scene_hierarchy_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "editor_context.h"
#include "scene/scene.h"
#include "scene/scene_node.h"

#include <imgui.h>
#include <string>

namespace RealmEngine
{
    SceneHierarchyWidget::SceneHierarchyWidget(std::shared_ptr<EditorContext> context, EditorEngineBridge& bridge) :
        Widget("Scene Hierarchy"), m_context(context), m_bridge(&bridge)
    {}

    void SceneHierarchyWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        auto current_scene = m_bridge->getCurrentScene();
        if (!current_scene)
        {
            ImGui::Text("No scene loaded");
            ImGui::End();
            return;
        }

        auto root = current_scene->getRoot();
        if (root)
            renderNode(root, *current_scene);

        ImGui::End();
    }

    void SceneHierarchyWidget::renderNode(std::shared_ptr<SceneNode> node, Scene& scene)
    {
        if (!node)
            return;

        std::string label = node->getName();
        if (node->hasEntity())
            label += " (Entity)";

        bool has_children = node->getChildCount() > 0;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!has_children)
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (m_context && m_context->hasSelectedNode() && m_context->getSelectedNode() == node)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool is_open = ImGui::TreeNodeEx(label.c_str(), flags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            if (m_context)
            {
                m_context->setSelectedNode(node);
                entt::entity entity = entt::null;
                if (node->hasEntity() && scene.valid(node->getEntity()))
                {
                    entity = node->getEntity();
                    m_context->setSelectedEntity(entity);
                }
                else
                    m_context->clearSelectedEntity();
                m_bridge->getEventBus().publish(EntitySelectedEvent {entity, node.get()});
            }
        }

        if (is_open)
        {
            for (size_t i = 0; i < node->getChildCount(); ++i)
            {
                auto child = node->getChild(i);
                if (child)
                    renderNode(child, scene);
            }
            ImGui::TreePop();
        }
    }

} // namespace RealmEngine
