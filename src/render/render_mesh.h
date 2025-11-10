#pragma once

#include "render/render_material.h"
#include "render/shader.h"
#include "render/vertex.h"
#include <glm/glm.hpp>
#include <vector>

namespace RealmEngine
{
    const int TEXTURE_UNIT_ALBEDO             = 0;
    const int TEXTURE_UNIT_METALLIC_ROUGHNESS = 1;
    const int TEXTURE_UNIT_NORMAL             = 2;
    const int TEXTURE_UNIT_AMBIENT_OCCLUSION  = 3;
    const int TEXTURE_UNIT_EMISSIVE           = 4;

    /**
     * A mesh is a collection of geometry paired with a material.
     */
    class RenderMesh
    {
    public:
        RenderMesh(std::vector<RenderVertex> vertices, std::vector<unsigned int> indices, RenderMaterial material);
        void draw(Shader& shader);

        std::vector<RenderVertex> m_vertices;
        std::vector<unsigned int> m_indices;
        RenderMaterial             m_material;

    private:
        void init();

        // OpenGL data structures
        unsigned int mVAO, mVBO, mEBO;
    };
} // namespace RealmEngine
