#include "render/cube.h"

#include <glad/gl.h>
#include <vector>

namespace RealmEngine
{
    Cube::Cube() { loadVertexData(); }

    void Cube::draw()
    {
        // draw mesh
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(mVertices.size() / 3));
        glBindVertexArray(0);
    }

    void Cube::loadVertexData()
    {
        // create our data structures
        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);

        glBindVertexArray(mVAO); // use this VAO for subsequent calls

        glBindBuffer(GL_ARRAY_BUFFER, mVBO); // use this VBO for subsequent calls
        glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), &mVertices[0], GL_STATIC_DRAW); // copy over the vertex data

        // setup the locations of vertex data
        // positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }
} // namespace RealmEngine

