#include "core/math/aabb.h"
#include <algorithm>
#include <cmath>

namespace RealmEngine
{
    bool AABB::intersectsRay(const glm::vec3& origin, const glm::vec3& direction) const
    {
        float tmin = std::numeric_limits<float>::lowest();
        float tmax = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i)
        {
            if (std::abs(direction[i]) < std::numeric_limits<float>::epsilon())
            {
                if (origin[i] < min[i] || origin[i] > max[i])
                    return false;
            }
            else
            {
                float inv_dir = 1.0f / direction[i];
                float t1      = (min[i] - origin[i]) * inv_dir;
                float t2      = (max[i] - origin[i]) * inv_dir;

                if (t1 > t2)
                    std::swap(t1, t2);

                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);

                if (tmin > tmax)
                    return false;
            }
        }

        return tmax >= 0.0f;
    }

    AABB AABB::transform(const glm::mat4& matrix) const
    {
        glm::vec3 new_min(std::numeric_limits<float>::max());
        glm::vec3 new_max(std::numeric_limits<float>::lowest());

        const glm::vec3 vertices[8] = {glm::vec3(min.x, min.y, min.z),
                                       glm::vec3(max.x, min.y, min.z),
                                       glm::vec3(min.x, max.y, min.z),
                                       glm::vec3(max.x, max.y, min.z),
                                       glm::vec3(min.x, min.y, max.z),
                                       glm::vec3(max.x, min.y, max.z),
                                       glm::vec3(min.x, max.y, max.z),
                                       glm::vec3(max.x, max.y, max.z)};

        for (const auto& vertex : vertices)
        {
            glm::vec4 transformed = matrix * glm::vec4(vertex, 1.0f);
            glm::vec3 transformed_vec3;

            if (std::abs(transformed.w) > std::numeric_limits<float>::epsilon())
            {
                transformed_vec3 = glm::vec3(transformed) / transformed.w;
            }
            else
            {
                transformed_vec3 = glm::vec3(transformed);
            }

            new_min = glm::min(new_min, transformed_vec3);
            new_max = glm::max(new_max, transformed_vec3);
        }

        return AABB(new_min, new_max);
    }

    bool AABB::intersectsSphere(const glm::vec3& center, float radius) const
    {
        float dist_sq = 0.0f;

        for (int i = 0; i < 3; ++i)
        {
            if (center[i] < min[i])
            {
                float diff = min[i] - center[i];
                dist_sq += diff * diff;
            }
            else if (center[i] > max[i])
            {
                float diff = center[i] - max[i];
                dist_sq += diff * diff;
            }
        }

        return dist_sq <= radius * radius;
    }

    float AABB::distanceSquared(const glm::vec3& point) const
    {
        float dist_sq = 0.0f;

        for (int i = 0; i < 3; ++i)
        {
            if (point[i] < min[i])
            {
                float diff = min[i] - point[i];
                dist_sq += diff * diff;
            }
            else if (point[i] > max[i])
            {
                float diff = point[i] - max[i];
                dist_sq += diff * diff;
            }
        }

        return dist_sq;
    }

} // namespace RealmEngine
