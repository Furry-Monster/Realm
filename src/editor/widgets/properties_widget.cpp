#include "editor/widgets/properties_widget.h"

#include "editor/editor_context.h"
#include "gameplay/components/lighting.h"
#include "gameplay/components/renderable.h"
#include "gameplay/components/transform.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RealmEngine
{
    PropertiesWidget::PropertiesWidget(EditorContext* context) : Widget("Properties"), m_context(context) {}

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

        if (entity->hasComponent<Lighting>())
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
            // Model path
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

            // Render object status
            if (renderable->hasRenderObject())
            {
                ImGui::Text("Render Object: Loaded");
            }
            else
            {
                ImGui::Text("Render Object: Not loaded");
            }
        }
    }

    void PropertiesWidget::renderLighting()
    {
        if (!m_context || !m_context->hasSelectedEntity())
            return;

        auto entity   = m_context->getSelectedEntity();
        auto lighting = entity->getComponent<Lighting>();
        if (!lighting)
            return;

        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Position
            glm::vec3 position = lighting->getPosition();
            if (ImGui::DragFloat3("Position", &position.x, 0.1f))
            {
                lighting->setPosition(position);
            }

            // Color
            glm::vec3 color = lighting->getColor();
            if (ImGui::ColorEdit3("Color", &color.x))
            {
                lighting->setColor(color);
            }
        }
    }

} // namespace RealmEngine
