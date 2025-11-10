#pragma once

#include <glm/glm.hpp>

namespace RealmEngine
{
    struct RenderVertex
    {
        glm::vec3 m_position;
        glm::vec3 m_normal;
        glm::vec2 m_texture_coordinates;
        glm::vec3 m_tangent;
        glm::vec3 m_bitangent;
    };
} // namespace RealmEngine
