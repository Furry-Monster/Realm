#pragma once

#include "gameplay/entity.h"
#include <glm/glm.hpp>
#include <vector>

namespace RealmEngine
{
    /**
     * A global object that represents all the things in our current scene.
     */
    class Scene
    {
    public:
        Scene()           = default;
        ~Scene() noexcept = default;

        Scene(const Scene&)                = delete;
        Scene& operator=(const Scene&)     = delete;
        Scene(Scene&&) noexcept            = default;
        Scene& operator=(Scene&&) noexcept = default;

        std::vector<Entity>    m_entities;
        std::vector<glm::vec3> m_light_positions;
        std::vector<glm::vec3> m_light_colors;
    };
} // namespace RealmEngine
