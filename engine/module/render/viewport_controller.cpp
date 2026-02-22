#include "module/render/viewport_controller.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "platform/input/input.h"

namespace RealmEngine
{
    void ViewportController::initialize(const std::shared_ptr<RenderCamera>& camera,
                                        Input&                               input,
                                        const float                          mouse_sensitivity,
                                        const float                          move_speed,
                                        const float                          sprint_multiplier)
    {
        m_camera            = camera;
        m_input             = &input;
        m_mouse_sensitivity = mouse_sensitivity;
        m_move_speed        = move_speed;
        m_sprint_multiplier = sprint_multiplier;
    }

    void ViewportController::update(const float delta_time)
    {
        if (!m_camera || !m_input)
            return;

        const Command cmd = m_input->getCurrentCommand();

        float move_speed = m_move_speed;
        if (cmd & static_cast<Command>(BindableCommand::SPRINT))
            move_speed *= m_sprint_multiplier;

        glm::vec3 move_dir(0.0f);

        if (cmd & static_cast<Command>(BindableCommand::FORWARD))
            move_dir += m_camera->getLocalForward();
        if (cmd & static_cast<Command>(BindableCommand::BACKWARD))
            move_dir -= m_camera->getLocalForward();
        if (cmd & static_cast<Command>(BindableCommand::LEFT))
            move_dir -= m_camera->getLocalRight();
        if (cmd & static_cast<Command>(BindableCommand::RIGHT))
            move_dir += m_camera->getLocalRight();

        if (glm::length(move_dir) > 0.0f)
        {
            move_dir                     = glm::normalize(move_dir);
            const glm::vec3 new_position = m_camera->getPosition() + move_dir * move_speed * delta_time;
            m_camera->setPosition(new_position);
        }

        if (cmd & static_cast<Command>(BindableCommand::FOCUS))
        {
            const float yaw_delta   = static_cast<float>(m_input->getCursorDeltaX()) * m_mouse_sensitivity;
            const float pitch_delta = static_cast<float>(m_input->getCursorDeltaY()) * m_mouse_sensitivity;

            const glm::quat current_rotation = m_camera->getRotation();

            const glm::quat yaw_rotation   = glm::angleAxis(glm::radians(-yaw_delta), glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::quat pitch_rotation = glm::angleAxis(glm::radians(-pitch_delta), glm::vec3(1.0f, 0.0f, 0.0f));

            const glm::quat new_rotation = yaw_rotation * current_rotation * pitch_rotation;
            m_camera->setRotation(new_rotation);
        }
    }
} // namespace RealmEngine
