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
        }
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
