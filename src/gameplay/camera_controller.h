#pragma once

#include <memory>
#include "render/render_camera.h"

namespace RealmEngine
{
    class CameraController
    {
    public:
        CameraController()           = default;
        ~CameraController() noexcept = default;

        CameraController(const CameraController&)                = delete;
        CameraController& operator=(const CameraController&)     = delete;
        CameraController(CameraController&&) noexcept            = default;
        CameraController& operator=(CameraController&&) noexcept = default;

        void initialize(std::shared_ptr<RenderCamera> camera);
        void update(float delta_time);

        void setMoveSpeed(float speed) { m_move_speed = speed; }
        void setSprintMultiplier(float multiplier) { m_sprint_multiplier = multiplier; }
        void setMouseSensitivity(float sensitivity) { m_mouse_sensitivity = sensitivity; }

        float getMoveSpeed() const { return m_move_speed; }
        float getSprintMultiplier() const { return m_sprint_multiplier; }
        float getMouseSensitivity() const { return m_mouse_sensitivity; }

    private:
        std::shared_ptr<RenderCamera> m_camera {nullptr};

        float m_move_speed {5.0f};
        float m_sprint_multiplier {2.0f};
        float m_mouse_sensitivity {0.1f};
    };

} // namespace RealmEngine
