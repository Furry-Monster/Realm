#pragma once

#include <vector>

namespace RealmEngine
{
    const unsigned int QUAD_NUM_TRIANGLES = 6;

    /**
     * A unit square that covers the whole screen.
     *
     * Includes texture coordinates.
     */
    class FullscreenQuad
    {
    public:
        FullscreenQuad();
        void draw();

    private:
        void loadVertexData();

        unsigned int mVAO;
        unsigned int mVBO;

        std::vector<float> mVertices = {
            // positions   // textureCoordinates
            -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};
    };
} // namespace RealmEngine

