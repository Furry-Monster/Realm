#pragma once

#include <glm/glm.hpp>

namespace RealmEngine
{
    struct AreaLight
    {
        glm::vec3 color {1.0f};
        float     intensity = 1.0f;
        bool      enabled   = true;
        float     width     = 1.0f;
        float     height    = 1.0f;
    };

} // namespace RealmEngine
