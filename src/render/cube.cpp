#include "render/cube.h"

#include <glad/gl.h>
#include <vector>

namespace RealmEngine
{
    Cube::Cube() { loadVertexData(); }

    void Cube::draw()
    {
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(mVertices.size() / 3));
        glBindVertexArray(0);
    }

    void Cube::loadVertexData()
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     mVertices.size() * sizeof(float),
                     &mVertices[0],
                     GL_STATIC_DRAW); // copy over the vertex data

        // setup the locations of vertex data
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

        glBindVertexArray(0);
    }
} // namespace RealmEngine
