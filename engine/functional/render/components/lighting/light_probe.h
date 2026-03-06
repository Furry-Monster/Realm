#pragma once

#include <array>
#include <glm/glm.hpp>

namespace RealmEngine
{
    struct LightProbe
    {
        std::array<glm::vec3, 9> sh_coefficients {};
        float                    influence_radius {10.0f};
        bool                     needs_update {true};
        bool                     enabled {true};
    };

} // namespace RealmEngine
