#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "gameplay/camera_controller.h"

namespace RealmEngine
{
    class RenderCamera;

    class Scene
    {
    public:
        Scene()           = default;
        ~Scene() noexcept = default;

        Scene(const Scene&)                = delete;
        Scene& operator=(const Scene&)     = delete;
        Scene(Scene&&) noexcept            = default;
        Scene& operator=(Scene&&) noexcept = default;

        void initialize(std::shared_ptr<RenderCamera> camera);
        void tick(float delta_time);

    private:
        std::shared_ptr<CameraController> m_camera_controller;
    };
} // namespace RealmEngine
