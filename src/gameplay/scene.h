#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "gameplay/camera_controller.h"
#include "gameplay/entity.h"

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

        std::vector<Entity>    m_entities;
        std::vector<glm::vec3> m_light_positions;
        std::vector<glm::vec3> m_light_colors;

    private:
        std::shared_ptr<CameraController> m_camera_controller;
    };
} // namespace RealmEngine
