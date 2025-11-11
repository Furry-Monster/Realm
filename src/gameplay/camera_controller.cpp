#include "camera_controller.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include "global_context.h"
#include "input.h"
#include "render/render_camera.h"

namespace RealmEngine
{
    void CameraController::initialize(std::shared_ptr<RenderCamera> camera) { m_camera = camera; }

    void CameraController::update(float delta_time)
    {
        if (!m_camera)
            return;

        std::shared_ptr<Input> input = g_context.m_input;
        Command                cmd   = input->getCurrentCommand();

        // Calculate movement speed
        float move_speed = m_move_speed;
        if (cmd & static_cast<Command>(BindableCommand::SPRINT))
        {
            move_speed *= m_sprint_multiplier;
        }

        // Calculate movement direction
        glm::vec3 move_dir(0.0f);

        if (cmd & static_cast<Command>(BindableCommand::FORWARD))
            move_dir += m_camera->getLocalForward();
        if (cmd & static_cast<Command>(BindableCommand::BACKWARD))
            move_dir -= m_camera->getLocalForward();
        if (cmd & static_cast<Command>(BindableCommand::LEFT))
            move_dir -= m_camera->getLocalRight();
        if (cmd & static_cast<Command>(BindableCommand::RIGHT))
            move_dir += m_camera->getLocalRight();

        // Apply movement
        if (glm::length(move_dir) > 0.0f)
        {
            move_dir               = glm::normalize(move_dir);
            glm::vec3 new_position = m_camera->getPosition() + move_dir * move_speed * delta_time;
            m_camera->setPosition(new_position);
        }

        // Mouse rotation control (when FOCUS is active)
        if (cmd & static_cast<Command>(BindableCommand::FOCUS))
        {
            float yaw_delta   = static_cast<float>(input->m_cursor_delta_x) * m_mouse_sensitivity;
            float pitch_delta = static_cast<float>(input->m_cursor_delta_y) * m_mouse_sensitivity;

            glm::quat current_rotation = m_camera->getRotation();

            // Apply yaw rotation (around world Y axis)
            glm::quat yaw_rotation = glm::angleAxis(glm::radians(-yaw_delta), glm::vec3(0.0f, 1.0f, 0.0f));
            // Apply pitch rotation (around local right axis)
            glm::quat pitch_rotation = glm::angleAxis(glm::radians(-pitch_delta), m_camera->getLocalRight());

            // Combine rotations: yaw first, then pitch
            glm::quat new_rotation = yaw_rotation * current_rotation * pitch_rotation;
            m_camera->setRotation(new_rotation);
        }
    }

} // namespace RealmEngine
