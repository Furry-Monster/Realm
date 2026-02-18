#pragma once

#include <glm/glm.hpp>

namespace RealmEngine
{
    struct DirectionalLight
    {
        glm::vec3 color {1.0f};
        float     intensity = 1.0f;
        bool      enabled   = true;
    };

} // namespace RealmEngine
