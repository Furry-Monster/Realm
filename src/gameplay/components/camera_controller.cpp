#include "camera_controller.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include "global_context.h"
#include "input.h"
#include "render/renderer.h"
#include "resource/config_manager.h"

namespace RealmEngine
{
    CameraController::CameraController()
    {
        m_camera = g_context.m_renderer->getCamera();

        const GamePlayConfig& gameplay_config = g_context.m_config->getGamePlayConfig();
        m_mouse_sensitivity                   = gameplay_config.camera_mouse_sensitivity;
        m_move_speed                          = gameplay_config.camera_move_speed;
        m_sprint_multiplier                   = gameplay_config.camera_sprint_multiplier;
    }

    void CameraController::update(float delta_time)
    {
        if (!m_camera)
            return;

        std::shared_ptr<Input> input = g_context.m_input;
        Command                cmd   = input->getCurrentCommand();

        // Calculate movement speed
        float move_speed = m_move_speed;
        if (cmd & static_cast<Command>(BindableCommand::SPRINT))
            move_speed *= m_sprint_multiplier;

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

            glm::quat yaw_rotation   = glm::angleAxis(glm::radians(-yaw_delta), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat pitch_rotation = glm::angleAxis(glm::radians(-pitch_delta), glm::vec3(1.0f, 0.0f, 0.0f));

            glm::quat new_rotation = yaw_rotation * current_rotation * pitch_rotation;
            m_camera->setRotation(new_rotation);
        }
    }

} // namespace RealmEngine
