#include "viewport_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "editor_context.h"
#include "module/ecs/components/hierarchy.h"
#include "module/ecs/components/transform.h"
#include "module/ecs/components/world_transform.h"
#include "render/rhi/rhi_texture.h"
#include "render/viewport_display_mode.h"
#include "resource/config_manager.h"
#include "scene/scene.h"

#include <ImGuizmo.h>
#include <glad/glad.h>
#include <imgui.h>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>

namespace RealmEngine
{
    static const char* displayModeLabels[] =
        {"Lit", "Albedo", "Normals", "Metallic", "Roughness", "Material AO", "Emissive", "AO", "Depth"};

    static const char* gbufferPreviewLabels[] = {"Final Output",
                                                 "GBuffer: Albedo+AO",
                                                 "GBuffer: Normal+Metallic",
                                                 "GBuffer: Emissive+Roughness",
                                                 "GBuffer: Depth"};

    ViewportWidget::ViewportWidget(EditorEngineBridge& bridge, const std::shared_ptr<EditorContext>& context) :
        Widget("Viewport"), m_bridge(&bridge), m_context(context)
    {
        m_bridge->setRenderToViewportTexture(true);
    }

    static void disableBlendCB(const ImDrawList*, const ImDrawCmd*) { glDisable(GL_BLEND); }
    static void enableBlendCB(const ImDrawList*, const ImDrawCmd*) { glEnable(GL_BLEND); }

    static void drawTexturePreview(const RHITexture* tex, const ImVec2 avail, const bool force_opaque = false)
    {
        if (!tex)
        {
            ImGui::TextDisabled("Texture not available");
            return;
        }

        const float w = static_cast<float>(tex->getWidth());
        const float h = static_cast<float>(tex->getHeight());

        if (w <= 0.0f || h <= 0.0f || avail.x <= 0.0f || avail.y <= 0.0f)
        {
            ImGui::TextDisabled("Viewport resizing...");
            return;
        }

        const float       scale = std::min(avail.x / w, avail.y / h);
        const ImVec2      display_size(w * scale, h * scale);
        const ImTextureID tid = static_cast<ImTextureID>(static_cast<intptr_t>(tex->getNativeHandle()));

        if (force_opaque)
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddCallback(disableBlendCB, nullptr);
            ImGui::Image(tid, display_size, ImVec2(0, 1), ImVec2(1, 0));
            dl->AddCallback(enableBlendCB, nullptr);
        }
        else
        {
            ImGui::Image(tid, display_size, ImVec2(0, 1), ImVec2(1, 0));
        }
    }

    void ViewportWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_name.c_str(), &m_open))
        {
            ImGui::End();
            return;
        }

        const bool is_deferred = m_bridge->getPipelineMode() == PipelineMode::Deferred;

        // Pipeline mode label
        if (is_deferred)
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "[Deferred]");
        else
            ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "[Forward]");
        ImGui::SameLine();

        // Gizmo mode indicator
        if (m_context)
        {
            const char* gizmo_label = "";
            switch (m_context->getGizmoOperation())
            {
                case GizmoOperation::None:
                    gizmo_label = "Select";
                    break;
                case GizmoOperation::Translate:
                    gizmo_label = "Move [W]";
                    break;
                case GizmoOperation::Rotate:
                    gizmo_label = "Rotate [E]";
                    break;
                case GizmoOperation::Scale:
                    gizmo_label = "Scale [R]";
                    break;
            }
            ImGui::TextDisabled("| QWER: %s", gizmo_label);
            ImGui::SameLine();
        }

        // Display mode combo
        const float combo_width = ImGui::GetContentRegionAvail().x * 0.4f;
        ImGui::SetNextItemWidth(combo_width);
        int mode = static_cast<int>(m_bridge->getViewportDisplayMode());
        if (ImGui::Combo("##DisplayMode", &mode, displayModeLabels, static_cast<int>(ViewportDisplayMode::Count)))
        {
            m_bridge->setViewportDisplayMode(static_cast<ViewportDisplayMode>(mode));
        }

        // G-Buffer preview selector (deferred only)
        if (is_deferred)
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(combo_width);
            ImGui::Combo("##GBufferPreview", &m_gbuffer_preview, gbufferPreviewLabels, 5);
        }
        else
        {
            m_gbuffer_preview = 0;
        }

        const ImVec2 avail = ImGui::GetContentRegionAvail();

        if (m_gbuffer_preview == 0)
        {
            drawTexturePreview(m_bridge->getViewportTexture(), avail);
        }
        else
        {
            const RHITexture* gbuf_tex = nullptr;
            switch (m_gbuffer_preview)
            {
                case 1:
                    gbuf_tex = m_bridge->getGBufferAlbedoModelID();
                    break;
                case 2:
                    gbuf_tex = m_bridge->getGBufferNormalMetallic();
                    break;
                case 3:
                    gbuf_tex = m_bridge->getGBufferEmissiveRoughness();
                    break;
                case 4:
                    gbuf_tex = m_bridge->getGBufferDepth();
                    break;
                default:
                    break;
            }
            drawTexturePreview(gbuf_tex, avail, true);
        }

        if (m_context && m_gbuffer_preview == 0)
        {
            const ImVec2 vp_min = ImGui::GetItemRectMin();
            const ImVec2 vp_max = ImGui::GetItemRectMax();
            const float  vp_w   = vp_max.x - vp_min.x;
            const float  vp_h   = vp_max.y - vp_min.y;

            if (m_context->getGizmoOperation() == GizmoOperation::None && ImGui::IsItemHovered() &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                const ImVec2 mouse = ImGui::GetMousePos();
                const auto   scene = m_bridge->getCurrentScene();
                const auto*  tex   = m_bridge->getViewportTexture();
                const int    rw    = tex ? tex->getWidth() : static_cast<int>(vp_w);
                const int    rh    = tex ? tex->getHeight() : static_cast<int>(vp_h);
                const auto   entity =
                    m_bridge->pickEntityAtViewport(vp_min.x, vp_min.y, vp_w, vp_h, mouse.x, mouse.y, rw, rh);
                if (scene && scene->valid(entity))
                {
                    const auto node = scene->findNodeByEntity(entity);
                    m_context->setSelectedEntity(entity);
                    m_context->setSelectedNode(node);
                    m_context->setGizmoOperation(GizmoOperation::Translate);
                    m_bridge->getEventBus().publish(EntitySelectedEvent {entity, node ? node.get() : nullptr});
                }
                else
                {
                    m_context->clearSelectedEntity();
                    m_context->clearSelectedNode();
                    m_bridge->getEventBus().publish(EntitySelectedEvent {entt::null, nullptr});
                }
            }

            const auto scene  = m_bridge->getCurrentScene();
            const auto entity = m_context->getSelectedEntity();
            const auto op     = m_context->getGizmoOperation();

            if (scene && scene->valid(entity) && scene->has<Transform>(entity) && op != GizmoOperation::None)
            {
                float view[16], proj[16];
                m_bridge->getCameraViewProj(view, proj);

                glm::mat4 parent_world(1.0f);
                if (const auto* parent_comp = scene->tryGet<Parent>(entity))
                {
                    const auto parent = parent_comp->entity;
                    if (scene->valid(parent) && scene->has<WorldTransform>(parent))
                        parent_world = scene->get<WorldTransform>(parent).matrix;
                }

                auto&     transform = scene->get<Transform>(entity);
                glm::mat4 world     = parent_world * transform.getModelMatrix();
                float     matrix[16];
                memcpy(matrix, glm::value_ptr(world), sizeof(matrix));

                ImGuizmo::SetDrawlist(nullptr);
                ImGuizmo::SetRect(vp_min.x, vp_min.y, vp_w, vp_h);
                ImGuizmo::SetOrthographic(false);

                ImGuizmo::OPERATION guizmo_op = ImGuizmo::TRANSLATE;
                switch (op)
                {
                    case GizmoOperation::Translate:
                        guizmo_op = ImGuizmo::TRANSLATE;
                        break;
                    case GizmoOperation::Rotate:
                        guizmo_op = ImGuizmo::ROTATE;
                        break;
                    case GizmoOperation::Scale:
                        guizmo_op = ImGuizmo::SCALE;
                        break;
                    default:
                        break;
                }

                if (ImGuizmo::Manipulate(view, proj, guizmo_op, ImGuizmo::LOCAL, matrix))
                {
                    const glm::mat4 new_world = glm::make_mat4(matrix);
                    const glm::mat4 new_local = glm::inverse(parent_world) * new_world;

                    switch (op)
                    {
                        case GizmoOperation::Translate: {
                            transform.position = glm::vec3(new_local[3][0], new_local[3][1], new_local[3][2]);
                            break;
                        }
                        case GizmoOperation::Rotate: {
                            transform.rotation = glm::quat_cast(glm::mat3(new_local));
                            break;
                        }
                        case GizmoOperation::Scale: {
                            const glm::mat3 rot_scale = glm::mat3(new_local);
                            transform.scale           = glm::vec3(
                                glm::length(rot_scale[0]), glm::length(rot_scale[1]), glm::length(rot_scale[2]));
                            break;
                        }
                        default:
                            break;
                    }
                    scene->markDirty();
                }
            }
        }

        ImGui::End();
    }

} // namespace RealmEngine
