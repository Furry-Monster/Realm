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
    };

    struct Attenuation
    {
        float constant;
        float linear;
        float quadratic;
        float range;
    };

    struct PointLight
        : public Light
        , public Attenuation
    {};

    struct SpotLight
        : public Light
        , public Attenuation
    {
        float inner_cone_angle;
        float outer_cone_angle;
    };

    struct AreaLight : public Light
    {
        float width;
        float height;
    };

} // namespace RealmEngine
