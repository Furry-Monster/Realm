#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include <glm/glm.hpp>

namespace RealmEngine
{
    struct DirectionalLight
    {
        glm::vec3 direction {0.0f, -1.0f, 0.0f};
        float     layout_pad0 {0.0f};

        glm::vec3 color {1.0f, 1.0f, 1.0f};
        float     intensity {1.0f};

        int       cast_shadow {1};
        glm::vec3 layout_pad1 {0.0f};

        glm::mat4 light_space_matrix {1.0f}; // For shadow mapping

        DirectionalLight() = default;
        DirectionalLight(const glm::vec3& dir, const glm::vec3& col, float inten) :
            direction(glm::normalize(dir)), color(col), intensity(inten)
        {}
    };

    struct PointLight
    {
        glm::vec3 position {0.0f, 0.0f, 0.0f};
        float     range {10.0f};

        glm::vec3 color {1.0f, 1.0f, 1.0f};
        float     intensity {1.0f};

        // attenuation = 1.0 / (constant + linear * d + quadratic * d^2)
        float constant {1.0f};
        float linear {0.09f};
        float quadratic {0.032f};
        int   cast_shadow {0};

        PointLight() = default;
        PointLight(const glm::vec3& pos, const glm::vec3& col, float inten, float r) :
            position(pos), range(r), color(col), intensity(inten)
        {}
    };

    struct SpotLight
    {
        glm::vec3 position {0.0f, 0.0f, 0.0f};
        float     range {10.0f};

        glm::vec3 direction {0.0f, -1.0f, 0.0f};
        float     intensity {1.0f};

        glm::vec3 color {1.0f, 1.0f, 1.0f};
        float     inner_cutoff {0.9f};

        float outer_cutoff {0.8f};
        float constant {1.0f};
        float linear {0.09f};
        float quadratic {0.032f};

        int       cast_shadow {0};
        glm::vec3 layout_pad0 {0.0f};

        glm::mat4 light_space_matrix {1.0f}; // For shadow mapping

        SpotLight() = default;
        SpotLight(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& col, float inten) :
            position(pos), direction(glm::normalize(dir)), intensity(inten), color(col)
        {}

        void setCutoffAngles(float inner_degrees, float outer_degrees)
        {
            inner_cutoff = glm::cos(glm::radians(inner_degrees));
            outer_cutoff = glm::cos(glm::radians(outer_degrees));
        }
    };

    struct AmbientLight
    {
        glm::vec3 color {0.03f, 0.03f, 0.03f};
        float     intensity {1.0f};
    };

} // namespace RealmEngine