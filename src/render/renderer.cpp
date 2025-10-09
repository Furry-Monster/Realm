#include "renderer.h"

namespace RealmEngine
{
    void Renderer::initialize() {}

    void Renderer::disposal() {}

    void Renderer::renderFrame()
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
} // namespace RealmEngine