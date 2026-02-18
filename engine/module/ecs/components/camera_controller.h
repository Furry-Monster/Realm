#pragma once

#include <memory>
#include "render/render_camera.h"

namespace RealmEngine
{
    class Input;

    class CameraController
    {
    public:
        CameraController()           = default;
        ~CameraController() noexcept = default;

        CameraController(const CameraController&)                = delete;
        CameraController& operator=(const CameraController&)     = delete;
        CameraController(CameraController&&) noexcept            = default;
        CameraController& operator=(CameraController&&) noexcept = default;

        void initialize(const std::shared_ptr<RenderCamera>& camera,
                        Input&                               input,
                        float                                mouse_sensitivity,
                        float                                move_speed,
                        float                                sprint_multiplier);

        void update(float delta_time);

        void setMoveSpeed(const float speed) { m_move_speed = speed; }
        void setSprintMultiplier(const float multiplier) { m_sprint_multiplier = multiplier; }
        void setMouseSensitivity(const float sensitivity) { m_mouse_sensitivity = sensitivity; }

        float getMoveSpeed() const { return m_move_speed; }
        float getSprintMultiplier() const { return m_sprint_multiplier; }
        float getMouseSensitivity() const { return m_mouse_sensitivity; }

    private:
        std::shared_ptr<RenderCamera> m_camera {nullptr};
        Input*                        m_input {nullptr};

        float m_move_speed {5.0f};
        float m_sprint_multiplier {2.0f};
        float m_mouse_sensitivity {0.1f};
    };

} // namespace RealmEngine
