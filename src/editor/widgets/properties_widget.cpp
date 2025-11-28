#include "editor/widgets/properties_widget.h"

#include "editor/editor_context.h"
#include "gameplay/components/lighting/point.h"
#include "gameplay/components/renderable.h"
#include "gameplay/components/transform.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RealmEngine
{
    PropertiesWidget::PropertiesWidget(std::shared_ptr<EditorContext> context) :
        Widget("Properties"), m_context(context)
    {}

    void PropertiesWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        if (!m_context || !m_context->hasSelectedEntity())
        {
            ImGui::Text("No entity selected");
            ImGui::End();
            return;
        }

        auto entity = m_context->getSelectedEntity();
        if (!entity)
        {
            ImGui::Text("Invalid entity");
            ImGui::End();
            return;
        }

        if (entity->hasComponent<Transform>())
            renderTransform();

        if (entity->hasComponent<Renderable>())
            renderRenderable();

        if (entity->hasComponent<Point>())
            renderLighting();

        ImGui::End();
    }

    void PropertiesWidget::renderTransform()
    {
        if (!m_context || !m_context->hasSelectedEntity())
            return;

        auto entity    = m_context->getSelectedEntity();
        auto transform = entity->getComponent<Transform>();
        if (!transform)
            return;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Position
            glm::vec3 position = transform->getPosition();
            if (ImGui::DragFloat3("Position", &position.x, 0.1f))
            {
                transform->setPosition(position);
            }

            // Rotation (Euler angles)
            glm::vec3 euler         = transform->getEulerAngles();
            glm::vec3 euler_degrees = glm::degrees(euler);
            if (ImGui::DragFloat3("Rotation", &euler_degrees.x, 1.0f))
            {
                transform->setRotation(glm::radians(euler_degrees));
            }

            // Scale
            glm::vec3 scale = transform->getScale();
            if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
            {
                transform->setScale(scale);
            }
        }
    }

    void PropertiesWidget::renderRenderable()
    {
        if (!m_context || !m_context->hasSelectedEntity())
            return;

        auto entity     = m_context->getSelectedEntity();
        auto renderable = entity->getComponent<Renderable>();
        if (!renderable)
            return;

        if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (renderable->hasModelPath())
            {
                std::string model_path = renderable->getModelPath();
                char        buffer[256];
                strncpy(buffer, model_path.c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';

                if (ImGui::InputText("Model Path", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly))
                {
                    // Read-only for now
                }
            }
            else
            {
                ImGui::Text("No model path");
            }

            if (renderable->hasRenderObject())
                ImGui::Text("Render Object: Loaded");
            else
                ImGui::Text("Render Object: Not loaded");
        }
    }

    void PropertiesWidget::renderLighting()
    {
        if (!m_context || !m_context->hasSelectedEntity())
            return;

        auto entity      = m_context->getSelectedEntity();
        auto point_light = entity->getComponent<Point>();
        if (!point_light)
            return;

        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool enabled = point_light->isEnabled();
            if (ImGui::Checkbox("Enabled", &enabled))
                point_light->setEnabled(enabled);

            glm::vec3 color = point_light->getColor();
            if (ImGui::ColorEdit3("Color", &color.x))
                point_light->setColor(color);

            float intensity = point_light->getIntensity();
            if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f))
                point_light->setIntensity(intensity);

            float range = point_light->getRange();
            if (ImGui::DragFloat("Range", &range, 1.0f, 0.0f, 1000.0f))
                point_light->setRange(range);

            if (ImGui::TreeNode("Attenuation"))
            {
                float constant  = point_light->getConstantAttenuation();
                float linear    = point_light->getLinearAttenuation();
                float quadratic = point_light->getQuadraticAttenuation();

                if (ImGui::DragFloat("Constant", &constant, 0.01f, 0.0f, 10.0f))
                    point_light->setConstantAttenuation(constant);
                if (ImGui::DragFloat("Linear", &linear, 0.001f, 0.0f, 1.0f))
                    point_light->setLinearAttenuation(linear);
                if (ImGui::DragFloat("Quadratic", &quadratic, 0.0001f, 0.0f, 1.0f))
                    point_light->setQuadraticAttenuation(quadratic);

                ImGui::TreePop();
            }
        }
    }

} // namespace RealmEngine
