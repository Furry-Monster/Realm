#include "renderer.h"

#include "utils.h"

namespace RealmEngine
{
    void Renderer::initialize() { info("renderer initalized."); }

    void Renderer::disposal() { info("renderer disposed all resources."); }

    void Renderer::renderFrame()
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // m_render_pipeline->render();
    }
} // namespace RealmEngine