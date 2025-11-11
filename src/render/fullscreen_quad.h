#pragma once

#include <vector>

namespace RealmEngine
{
    const unsigned int QUAD_NUM_TRIANGLES = 6;

    class FullscreenQuad
    {
    public:
        FullscreenQuad();
        void draw() const;

    private:
        void loadVertexData();

        unsigned int m_vao;
        unsigned int m_vbo;

        std::vector<float> m_vertices = {-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                                         -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};
    };
} // namespace RealmEngine
