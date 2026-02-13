#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "renderer/render_material.h"
#include "renderer/shader.h"
#include "renderer/vertex.h"

namespace RealmEngine
{
    class RenderMesh
    {
    public:
        RenderMesh(std::vector<RenderVertex> vertices, std::vector<unsigned int> indices, RenderMaterial material);

        void draw(Shader& shader);

        std::vector<RenderVertex> m_vertices;
        std::vector<unsigned int> m_indices;
        RenderMaterial            m_material;

    private:
        unsigned int m_vao, m_vbo, m_ebo;
    };
} // namespace RealmEngine
