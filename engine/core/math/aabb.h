#pragma once

#include <limits>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

namespace RealmEngine
{
    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;

        constexpr AABB() : min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest()) {}

        constexpr AABB(const glm::vec3& min_point, const glm::vec3& max_point) : min(min_point), max(max_point) {}

        constexpr AABB(const glm::vec3& center, const float half_size) :
            min(center - glm::vec3(half_size)), max(center + glm::vec3(half_size))
        {}

        constexpr glm::vec3 center() const { return (min + max) * 0.5f; }
        constexpr glm::vec3 extent() const { return max - min; }

        constexpr float width() const { return max.x - min.x; }
        constexpr float height() const { return max.y - min.y; }
        constexpr float depth() const { return max.z - min.z; }

        constexpr bool isValid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }

        constexpr bool isEmpty() const { return min.x >= max.x || min.y >= max.y || min.z >= max.z; }
        constexpr bool contains(const glm::vec3& point) const
        {
            return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y && point.z >= min.z &&
                   point.z <= max.z;
        }

        constexpr bool contains(const AABB& other) const
        {
            return min.x <= other.min.x && max.x >= other.max.x && min.y <= other.min.y && max.y >= other.max.y &&
                   min.z <= other.min.z && max.z >= other.max.z;
        }

        constexpr bool intersects(const AABB& other) const
        {
            return min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y && max.y >= other.min.y &&
                   min.z <= other.max.z && max.z >= other.min.z;
        }

        bool intersectsSphere(const glm::vec3& center, float radius) const;

        bool intersectsRay(const glm::vec3& origin, const glm::vec3& direction) const;

        constexpr bool intersectsPlane(const glm::vec4& plane) const
        {
            glm::vec3 positive_vert = min;
            if (plane.x >= 0)
                positive_vert.x = max.x;
            if (plane.y >= 0)
                positive_vert.y = max.y;
            if (plane.z >= 0)
                positive_vert.z = max.z;

            glm::vec3 negative_vert = max;
            if (plane.x >= 0)
                negative_vert.x = min.x;
            if (plane.y >= 0)
                negative_vert.y = min.y;
            if (plane.z >= 0)
                negative_vert.z = min.z;

            const float positive_dist = glm::dot(glm::vec3(plane), positive_vert) + plane.w;
            const float negative_dist = glm::dot(glm::vec3(plane), negative_vert) + plane.w;

            return positive_dist * negative_dist <= 0.0f;
        }

        void merge(const glm::vec3& point)
        {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        void merge(const AABB& other)
        {
            min = glm::min(min, other.min);
            max = glm::max(max, other.max);
        }

        static AABB merge(const AABB& a, const AABB& b)
        {
            AABB result = a;
            result.merge(b);
            return result;
        }

        void expand(const float amount)
        {
            const glm::vec3 offset(amount);
            min -= offset;
            max += offset;
        }

        void expand(const glm::vec3& amount)
        {
            min -= amount;
            max += amount;
        }

        void shrink(const float amount)
        {
            const glm::vec3 offset(amount);
            min += offset;
            max -= offset;
            if (!isValid())
            {
                const glm::vec3 c = center();
                min = max = c;
            }
        }

        void shrink(const glm::vec3& amount)
        {
            min += amount;
            max -= amount;
            if (!isValid())
            {
                const glm::vec3 c = center();
                min = max = c;
            }
        }

        AABB transform(const glm::mat4& matrix) const;

        void translate(const glm::vec3& offset)
        {
            min += offset;
            max += offset;
        }

        void scale(const float factor)
        {
            const glm::vec3 c = center();
            const glm::vec3 e = extent() * factor;
            min               = c - e * 0.5f;
            max               = c + e * 0.5f;
        }

        void scale(const glm::vec3& factors)
        {
            const glm::vec3 c = center();
            const glm::vec3 e = extent() * factors;
            min               = c - e * 0.5f;
            max               = c + e * 0.5f;
        }

        void reset()
        {
            min = glm::vec3(std::numeric_limits<float>::max());
            max = glm::vec3(std::numeric_limits<float>::lowest());
        }

        glm::vec3 clampPoint(const glm::vec3& point) const { return glm::clamp(point, min, max); }

        float distanceSquared(const glm::vec3& point) const;
    };

} // namespace RealmEngine
