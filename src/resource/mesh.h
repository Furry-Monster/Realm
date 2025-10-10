#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

namespace RealmEngine
{
    /**
     * @brief PBR supported vertex structrue
     *
     */
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coord; // uv coord
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec4 color;
    };

    class Mesh
    {
    public:
    };
} // namespace RealmEngine