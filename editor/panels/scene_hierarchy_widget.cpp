#include "panels/scene_hierarchy_widget.h"

#include "editor_context.h"
#include "scene/scene_manager.h"
#include "scene/scene_node.h"
#include "global_context.h"

#include <imgui.h>
#include <string>

namespace RealmEngine
{
    SceneHierarchyWidget::SceneHierarchyWidget(std::shared_ptr<EditorContext> context) :
        Widget("Scene Hierarchy"), m_context(context)
    {}

    void SceneHierarchyWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        auto current_scene = g_context.m_scene->getCurrentScene();
        if (!current_scene)
        {
            ImGui::Text("No scene loaded");
            ImGui::End();
            return;
        }

        auto root = current_scene->getRoot();
        if (root)
            renderNode(root);

        ImGui::End();
    }

    void SceneHierarchyWidget::renderNode(std::shared_ptr<SceneNode> node)
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
                if (node->hasEntity())
                {
                    auto entity = g_context.m_scene->getCurrentScene()->getEntity(node->getEntityId());
                    m_context->setSelectedEntity(entity);
                }
                else
                {
                    m_context->clearSelectedEntity();
                }
            }
        }

        if (is_open)
        {
            for (size_t i = 0; i < node->getChildCount(); ++i)
            {
                auto child = node->getChild(i);
                if (child)
                    renderNode(child);
            }
            ImGui::TreePop();
        }
    }

} // namespace RealmEngine
