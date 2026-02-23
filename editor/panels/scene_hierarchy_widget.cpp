#include "panels/scene_hierarchy_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "editor_context.h"
#include "functional/ecs/components/transform.h"
#include "functional/ecs/entity.h"
#include "functional/scene/scene.h"
#include "functional/scene/scene_node.h"
#include "module/audio/components/audio_listener.h"
#include "module/audio/components/audio_source.h"
#include "module/camera/components/camera.h"
#include "module/render/components/lighting/area.h"
#include "module/render/components/lighting/directional.h"
#include "module/render/components/lighting/point.h"
#include "module/render/components/lighting/spot.h"
#include "panels/asset_browser_widget.h"

#include <imgui.h>
#include <algorithm>
#include <cstring>
#include <glm/glm.hpp>
#include <string>

namespace RealmEngine
{
    static constexpr const char* HIERARCHY_NODE_PAYLOAD = "SCENE_HIERARCHY_NODE";

    SceneHierarchyWidget::SceneHierarchyWidget(const std::shared_ptr<EditorContext>& context,
                                               EditorEngineBridge&                   bridge,
                                               HierarchyCallbacks                    callbacks) :
        Widget("Scene Hierarchy"), m_context(context), m_bridge(&bridge), m_callbacks(std::move(callbacks))
    {
        m_rename_buffer[0] = '\0';
    }

    void SceneHierarchyWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        const auto current_scene = m_bridge->getCurrentScene();
        if (!current_scene)
        {
            ImGui::Text("No scene loaded");
            ImGui::End();
            return;
        }

        if (ImGui::BeginChild("SceneHierarchyContent", ImVec2(0, 0), true))
        {
            const auto root = current_scene->getRoot();
            if (root)
                renderNode(root, *current_scene);

            if (ImGui::BeginPopupContextWindow("SceneHierarchyCreate", ImGuiPopupFlags_MouseButtonRight))
            {
                renderCreateEntityMenu(*current_scene, root);
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(AssetBrowserWidget::DRAG_DROP_PAYLOAD_TYPE))
            {
                const auto* const path_cstr = static_cast<const char*>(payload->Data);
                if (path_cstr)
                {
                    const std::filesystem::path path(path_cstr);
                    std::string                 ext = path.extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".gltf" || ext == ".glb" || ext == ".fbx" || ext == ".obj")
                        m_bridge->addModelToScene(path);
                    // TODO: fallback
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::End();
    }

    static bool isDescendantOf(const std::shared_ptr<SceneNode>& node, const std::shared_ptr<SceneNode>& ancestor)
    {
        auto p = node->getParent();
        while (p)
        {
            if (p == ancestor)
                return true;
            p = p->getParent();
        }
        return false;
    }

    void SceneHierarchyWidget::renderNode(const std::shared_ptr<SceneNode>& node, Scene& scene)
    {
        if (!node)
            return;

        const auto  root         = scene.getRoot();
        const bool  is_root      = (node == root);
        std::string display_name = node->getName();
        if (node->hasEntity())
            display_name += " (Entity)";

        const bool         has_children = node->getChildCount() > 0;
        ImGuiTreeNodeFlags flags        = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!has_children)
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (m_context && m_context->hasSelectedNode() && m_context->getSelectedNode() == node)
            flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::PushID(node.get());

        const bool is_renaming = (m_renaming_node.lock() == node);

        if (is_renaming)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx("##rename_row", flags);
            ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##rename_input",
                                 m_rename_buffer,
                                 sizeof(m_rename_buffer),
                                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                node->setName(m_rename_buffer);
                m_renaming_node.reset();
                scene.markDirty();
            }
            if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
                m_renaming_node.reset();
        }
        else
        {
            const bool is_open = ImGui::TreeNodeEx(display_name.c_str(), flags);

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                ImGui::SetDragDropPayload(HIERARCHY_NODE_PAYLOAD, &node, sizeof(node));
                ImGui::Text("%s", display_name.c_str());
                ImGui::TextDisabled("Drop: Child | Shift+Drop: Reorder");
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(HIERARCHY_NODE_PAYLOAD))
                {
                    const auto dragged = *static_cast<const std::shared_ptr<SceneNode>*>(payload->Data);
                    if (dragged && dragged != node && !isDescendantOf(node, dragged))
                    {
                        const auto target_parent     = node->getParent();
                        const auto dragged_parent    = dragged->getParent();
                        const bool shift_for_reorder = (ImGui::GetIO().KeyMods & ImGuiMod_Shift) != 0;

                        if (shift_for_reorder && target_parent && target_parent == dragged_parent)
                        {
                            size_t target_idx  = 0;
                            size_t dragged_idx = target_parent->getChildCount();
                            for (size_t i = 0; i < target_parent->getChildCount(); ++i)
                            {
                                if (target_parent->getChild(i) == node)
                                    target_idx = i;
                                if (target_parent->getChild(i) == dragged)
                                    dragged_idx = i;
                            }
                            size_t insert_idx = (dragged_idx < target_idx) ? target_idx - 1 : target_idx;
                            target_parent->insertChildAt(insert_idx, dragged);
                        }
                        else
                        {
                            node->addChild(dragged);
                        }
                        scene.markDirty();
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (!is_root)
                    {
                        m_renaming_node = node;
                        strncpy(m_rename_buffer, node->getName().c_str(), sizeof(m_rename_buffer) - 1);
                        m_rename_buffer[sizeof(m_rename_buffer) - 1] = '\0';
                    }
                }
                else if (m_context)
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

            if (ImGui::BeginPopupContextItem())
            {
                if (m_context)
                {
                    m_context->setSelectedNode(node);
                    if (node->hasEntity() && scene.valid(node->getEntity()))
                        m_context->setSelectedEntity(node->getEntity());
                    else
                        m_context->clearSelectedEntity();
                }
                if (ImGui::MenuItem("Rename") && !is_root)
                {
                    m_renaming_node = node;
                    strncpy(m_rename_buffer, node->getName().c_str(), sizeof(m_rename_buffer) - 1);
                    m_rename_buffer[sizeof(m_rename_buffer) - 1] = '\0';
                }
                if (ImGui::MenuItem("Create Empty Child"))
                {
                    const auto new_node = scene.createNodeWithEntity("Entity");
                    auto       e        = scene.entity(new_node->getEntity());
                    e.emplace<Transform>();
                    node->addChild(new_node);
                    scene.markDirty();
                    if (m_context)
                    {
                        m_context->setSelectedNode(new_node);
                        m_context->setSelectedEntity(e.handle());
                        m_bridge->getEventBus().publish(EntitySelectedEvent {e.handle(), new_node.get()});
                    }
                }
                if (ImGui::BeginMenu("Create"))
                {
                    renderCreateEntityMenu(scene, node);
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Delete", "Del") && m_callbacks.on_delete)
                    m_callbacks.on_delete();
                if (ImGui::MenuItem("Duplicate", "Ctrl+D") && m_callbacks.on_duplicate)
                    m_callbacks.on_duplicate();
                ImGui::EndPopup();
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

        ImGui::PopID();
    }

    void SceneHierarchyWidget::renderCreateEntityMenu(Scene& scene, const std::shared_ptr<SceneNode>& parent)
    {
        if (!parent)
            return;

        auto add_as_child = [&](const std::string& base_name, auto&& setup) {
            const auto new_node = scene.createNodeWithEntity(base_name);
            auto       e        = scene.entity(new_node->getEntity());
            e.emplace<Transform>();
            setup(e);
            parent->addChild(new_node);
            scene.markDirty();
            if (m_context)
            {
                m_context->setSelectedNode(new_node);
                m_context->setSelectedEntity(e.handle());
                m_bridge->getEventBus().publish(EntitySelectedEvent {e.handle(), new_node.get()});
            }
        };

        if (ImGui::MenuItem("Create Empty"))
        {
            add_as_child("Entity", [](Entity&) {});
        }
        if (ImGui::MenuItem("Camera"))
        {
            add_as_child("Camera", [](Entity& e) {
                e.get<Transform>().position = glm::vec3(0.0f, 0.0f, 5.0f);
                e.emplace<Camera>();
                e.emplace<AudioListener>();
            });
        }
        if (ImGui::MenuItem("Point Light"))
        {
            add_as_child("PointLight", [](Entity& e) {
                auto& pl     = e.emplace<PointLight>();
                pl.color     = glm::vec3(1.0f);
                pl.intensity = 5.0f;
                pl.range     = 50.0f;
            });
        }
        if (ImGui::MenuItem("Spot Light"))
        {
            add_as_child("SpotLight", [](Entity& e) {
                auto& sl            = e.emplace<SpotLight>();
                sl.color            = glm::vec3(1.0f);
                sl.intensity        = 8.0f;
                sl.range            = 25.0f;
                sl.inner_cone_angle = 12.0f;
                sl.outer_cone_angle = 28.0f;
            });
        }
        if (ImGui::MenuItem("Directional Light"))
        {
            add_as_child("DirectionalLight", [](Entity& e) {
                auto& dl     = e.emplace<DirectionalLight>();
                dl.color     = glm::vec3(1.0f, 0.98f, 0.88f);
                dl.intensity = 3.5f;
            });
        }
        if (ImGui::MenuItem("Area Light"))
        {
            add_as_child("AreaLight", [](Entity& e) {
                auto& al     = e.emplace<AreaLight>();
                al.color     = glm::vec3(0.95f, 1.0f, 1.0f);
                al.intensity = 3.0f;
                al.width     = 2.5f;
                al.height    = 2.5f;
            });
        }
        if (ImGui::MenuItem("Audio Source"))
        {
            add_as_child("AudioSource", [](Entity& e) { e.emplace<AudioSource>(); });
        }
    }

} // namespace RealmEngine
