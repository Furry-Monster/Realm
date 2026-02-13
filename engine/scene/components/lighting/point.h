#pragma once

#include <glm/glm.hpp>

namespace RealmEngine
{
    struct PointLight
    {
        glm::vec3 color {1.0f};
        float     intensity = 1.0f;
        bool      enabled   = true;
        float     constant  = 1.0f;
        float     linear    = 0.09f;
        float     quadratic = 0.032f;
        float     range     = 50.0f;
    };

} // namespace RealmEngine
