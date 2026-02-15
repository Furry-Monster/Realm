#include "panels/properties_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "editor_context.h"
#include "scene/components/lighting/area.h"
#include "scene/components/lighting/directional.h"
#include "scene/components/lighting/point.h"
#include "scene/components/lighting/spot.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/scene.h"

#include "renderer/render_mesh.h"
#include "renderer/render_object.h"
#include "rhi/rhi_texture.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RealmEngine
{
    PropertiesWidget::PropertiesWidget(std::shared_ptr<EditorContext> context, EditorEngineBridge& bridge) :
        Widget("Properties"), m_context(context), m_bridge(&bridge)
    {}

    void PropertiesWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        auto scene = m_bridge->getCurrentScene();
        if (!m_context || !m_context->hasSelectedEntity() || !scene)
        {
            ImGui::Text("No entity selected");
            ImGui::End();
            return;
        }

        entt::entity entity = m_context->getSelectedEntity();
        if (!scene->valid(entity))
        {
            ImGui::Text("Invalid entity");
            ImGui::End();
            return;
        }

        if (scene->has<Transform>(entity))
            renderTransform();
        if (scene->has<Renderable>(entity))
            renderRenderable();
        if (scene->has<PointLight>(entity))
            renderPointLight();
        if (scene->has<SpotLight>(entity))
            renderSpotLight();
        if (scene->has<DirectionalLight>(entity))
            renderDirectionalLight();
        if (scene->has<AreaLight>(entity))
            renderAreaLight();

        ImGui::End();
    }

    void PropertiesWidget::renderTransform()
    {
        auto scene = m_bridge->getCurrentScene();
        if (!scene || !m_context->hasSelectedEntity())
            return;

        auto* tf = scene->tryGet<Transform>(m_context->getSelectedEntity());
        if (!tf)
            return;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::DragFloat3("Position", &tf->position.x, 0.1f))
            {
            }

            glm::vec3 euler_degrees = glm::degrees(tf->getEulerAngles());
            if (ImGui::DragFloat3("Rotation", &euler_degrees.x, 1.0f))
                tf->rotation = glm::quat(glm::radians(euler_degrees));

            if (ImGui::DragFloat3("Scale", &tf->scale.x, 0.1f))
            {
            }
        }
    }

    void PropertiesWidget::renderRenderable()
    {
        auto scene = m_bridge->getCurrentScene();
        if (!scene || !m_context->hasSelectedEntity())
            return;

        auto* r = scene->tryGet<Renderable>(m_context->getSelectedEntity());
        if (!r)
            return;

        if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (!r->model_path.empty())
            {
                char buffer[256];
                strncpy(buffer, r->model_path.c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                ImGui::InputText("Model Path", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
            }
            else
            {
                ImGui::Text("No model path");
            }

            ImGui::Text(r->render_object ? "Render Object: Loaded" : "Render Object: Not loaded");

            if (r->render_object && ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const size_t mesh_count = r->render_object->getMeshCount();
                for (size_t i = 0; i < mesh_count; ++i)
                {
                    RenderMesh* mesh = r->render_object->getMesh(i);
                    if (!mesh)
                        continue;

                    ImGui::PushID(static_cast<int>(i));
                    if (ImGui::TreeNodeEx(("Mesh " + std::to_string(i)).c_str(),
                                          ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text("Vertices: %zu", mesh->m_vertices.size());
                        ImGui::Text("Indices: %zu", mesh->m_indices.size());
                        ImGui::Text("Triangles: %d", mesh->getTriangleCount());

                        if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            renderMaterialPreview(mesh->m_material);
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        }
    }

    void PropertiesWidget::renderTextureSlot(const char* label, bool use_tex,
                                             const std::shared_ptr<RHITexture>& tex)
    {
        ImGui::Text("%s:", label);
        ImGui::SameLine(140);
        if (tex)
        {
            ImGui::Text("(%dx%d) %s", tex->getWidth(), tex->getHeight(), use_tex ? "on" : "off");
            const float preview_sz = 48.0f;
            ImTextureID tid = static_cast<ImTextureID>(static_cast<intptr_t>(tex->getNativeHandle()));
            ImGui::Image(tid, ImVec2(preview_sz, preview_sz), ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImGui::TextDisabled("[none] %s", use_tex ? "on" : "off");
        }
    }

    void PropertiesWidget::renderMaterialPreview(const RenderMaterial& mat)
    {
        ImGui::Text("Albedo: (%.2f, %.2f, %.2f)", mat.albedo.x, mat.albedo.y, mat.albedo.z);
        ImGui::Text("Metallic: %.2f", mat.metallic);
        ImGui::Text("Roughness: %.2f", mat.roughness);
        ImGui::Text("Ambient Occlusion: %.2f", mat.ambient_occlusion);
        ImGui::Text("Emissive: (%.2f, %.2f, %.2f)", mat.emissive.x, mat.emissive.y, mat.emissive.z);

        ImGui::Separator();
        ImGui::Text("Textures");
        renderTextureSlot("Albedo", mat.use_texture_albedo, mat.texture_albedo);
        renderTextureSlot("Metallic/Roughness", mat.use_texture_metallic_roughness,
                         mat.texture_metallic_roughness);
        renderTextureSlot("Normal", mat.use_texture_normal, mat.texture_normal);
        renderTextureSlot("Ambient Occlusion", mat.use_texture_ambient_occlusion,
                         mat.texture_ambient_occlusion);
        renderTextureSlot("Emissive", mat.use_texture_emissive, mat.texture_emissive);
    }

    void PropertiesWidget::renderPointLight()
    {
        auto scene = m_bridge->getCurrentScene();
        if (!scene || !m_context->hasSelectedEntity())
            return;

        auto* pl = scene->tryGet<PointLight>(m_context->getSelectedEntity());
        if (!pl)
            return;

        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &pl->enabled))
                scene->markDirty();
            ImGui::ColorEdit3("Color", &pl->color.x);
            ImGui::DragFloat("Intensity", &pl->intensity, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Range", &pl->range, 1.0f, 0.0f, 1000.0f);

            if (ImGui::TreeNode("Attenuation"))
            {
                ImGui::DragFloat("Constant", &pl->constant, 0.01f, 0.0f, 10.0f);
                ImGui::DragFloat("Linear", &pl->linear, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("Quadratic", &pl->quadratic, 0.0001f, 0.0f, 1.0f);
                ImGui::TreePop();
            }
        }
    }

    void PropertiesWidget::renderSpotLight()
    {
        auto scene = m_bridge->getCurrentScene();
        if (!scene || !m_context->hasSelectedEntity())
            return;

        auto* sl = scene->tryGet<SpotLight>(m_context->getSelectedEntity());
        if (!sl)
            return;

        if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &sl->enabled))
                scene->markDirty();
            ImGui::ColorEdit3("Color", &sl->color.x);
            ImGui::DragFloat("Intensity", &sl->intensity, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Range", &sl->range, 1.0f, 0.0f, 1000.0f);
            ImGui::DragFloat("Inner Cone Angle", &sl->inner_cone_angle, 1.0f, 0.0f, 180.0f);
            ImGui::DragFloat("Outer Cone Angle", &sl->outer_cone_angle, 1.0f, 0.0f, 180.0f);

            if (ImGui::TreeNode("Attenuation"))
            {
                ImGui::DragFloat("Constant", &sl->constant, 0.01f, 0.0f, 10.0f);
                ImGui::DragFloat("Linear", &sl->linear, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("Quadratic", &sl->quadratic, 0.0001f, 0.0f, 1.0f);
                ImGui::TreePop();
            }
        }
    }

    void PropertiesWidget::renderDirectionalLight()
    {
        auto scene = m_bridge->getCurrentScene();
        if (!scene || !m_context->hasSelectedEntity())
            return;

        auto* dl = scene->tryGet<DirectionalLight>(m_context->getSelectedEntity());
        if (!dl)
            return;

        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &dl->enabled))
                scene->markDirty();
            ImGui::ColorEdit3("Color", &dl->color.x);
            ImGui::DragFloat("Intensity", &dl->intensity, 0.1f, 0.0f, 100.0f);
        }
    }

    void PropertiesWidget::renderAreaLight()
    {
        auto scene = m_bridge->getCurrentScene();
        if (!scene || !m_context->hasSelectedEntity())
            return;

        auto* al = scene->tryGet<AreaLight>(m_context->getSelectedEntity());
        if (!al)
            return;

        if (ImGui::CollapsingHeader("Area Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Checkbox("Enabled", &al->enabled))
                scene->markDirty();
            ImGui::ColorEdit3("Color", &al->color.x);
            ImGui::DragFloat("Intensity", &al->intensity, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Width", &al->width, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Height", &al->height, 0.1f, 0.0f, 100.0f);
        }
    }

} // namespace RealmEngine
