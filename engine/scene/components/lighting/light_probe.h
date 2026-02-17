#pragma once

#include <array>
#include <glm/glm.hpp>

namespace RealmEngine
{
    struct LightProbe
    {
        // L2 SH coefficients (3 bands = 9 coefficients, RGB per coefficient)
        std::array<glm::vec3, 9> sh_coefficients {};
        float                    influence_radius {10.0f};
        bool                     needs_update {true};
        bool                     enabled {true};
    };

} // namespace RealmEngine
