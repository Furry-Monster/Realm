#pragma once

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "math.h"
#include <cstdint>
namespace RealmEngine
{
    struct Frusum
    {
        glm::vec4 planes[6];

        constexpr bool containsPoint(const glm::vec3& point) const;
        constexpr bool containsSphere(const glm::vec3& center, float r) const;
        constexpr bool containsAABB(const AABB& bounds) const;
    };

    enum class ProjectionType : uint8_t
    {
        Perspective,
        Orthographic
    };

    class RenderCamera
    {
    public:
        RenderCamera()           = default;
        ~RenderCamera() noexcept = default;

        RenderCamera(const RenderCamera&)                = delete;
        RenderCamera& operator=(const RenderCamera&)     = delete;
        RenderCamera(RenderCamera&&) noexcept            = default;
        RenderCamera& operator=(RenderCamera&&) noexcept = default;

    private:
    };

} // namespace RealmEngine