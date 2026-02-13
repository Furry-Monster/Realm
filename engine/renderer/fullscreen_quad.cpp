#include "renderer/fullscreen_quad.h"

#include <glad/gl.h>
#include <vector>

namespace RealmEngine
{
    FullscreenQuad::FullscreenQuad()
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices.size() * sizeof(float),
                     &m_vertices[0],
                     GL_STATIC_DRAW); // copy over the vertex data

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    FullscreenQuad::~FullscreenQuad()
    {
        if (m_vao != 0)
            glDeleteVertexArrays(1, &m_vao);
        if (m_vbo != 0)
            glDeleteBuffers(1, &m_vbo);
    }

    void FullscreenQuad::draw() const
    {
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, QUAD_NUM_TRIANGLES);
        glBindVertexArray(0);
    }
} // namespace RealmEngine
