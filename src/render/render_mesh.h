#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "render/render_material.h"
#include "render/shader.h"
#include "render/vertex.h"

namespace RealmEngine
{
    const int TEXTURE_UNIT_ALBEDO             = 0;
    const int TEXTURE_UNIT_METALLIC_ROUGHNESS = 1;
    const int TEXTURE_UNIT_NORMAL             = 2;
    const int TEXTURE_UNIT_AMBIENT_OCCLUSION  = 3;
    const int TEXTURE_UNIT_EMISSIVE           = 4;

    class RenderMesh
    {
    public:
        RenderMesh(std::vector<RenderVertex> vertices, std::vector<unsigned int> indices, RenderMaterial material);

        void draw(Shader& shader);

        std::vector<RenderVertex> m_vertices;
        std::vector<unsigned int> m_indices;
        RenderMaterial            m_material;

    private:
        void init();

        unsigned int m_vao, m_vbo, m_ebo;
    };
} // namespace RealmEngine
