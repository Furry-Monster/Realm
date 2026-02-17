#pragma once

#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

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

    struct alignas(16) LightData
    {
        glm::vec4 position;    // xyz = position, w = type
        glm::vec4 direction;   // xyz = direction, w = intensity
        glm::vec4 color;       // rgb = color, w = constant
        glm::vec4 attenuation; // x = linear, y = quadratic, z = range, w = inner_cone_angle
        glm::vec4 spot_area;   // x = outer_cone_angle, y = width, z = height, w = padding
    };

    static constexpr size_t   MAX_LIGHTS              = 16;
    static constexpr size_t   BUFFER_SIZE             = 16 + MAX_LIGHTS * sizeof(LightData);
    static constexpr uint32_t LIGHT_UBO_BINDING_POINT = 0;

} // namespace RealmEngine
