#pragma once

#include <cstdint>
#include "glm/ext/vector_float3.hpp"

namespace RealmEngine
{
    enum class LightType : uint8_t
    {
        Point,
        Directional,
        Spot,
        Area
    };

    struct Light
    {
        LightType type;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float     intensity;

        // Attenuation (for Point and Spot lights)
        float constant;
        float linear;
        float quadratic;
        float range;

        // Spot light parameters
        float inner_cone_angle;
        float outer_cone_angle;

        // Area light parameters
        float width;
        float height;
    };

} // namespace RealmEngine
