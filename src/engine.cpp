#include "engine.h"
#include "global_context.h"
#include "render/renderer.h"
#include "utils.h"
#include "window.h"

namespace RealmEngine
{
    void Engine::boot()
    {
        g_context.create();
        info("Boot Engine.");
    }

    void Engine::run()
    {
        while (!g_context.m_window->shouldClose())
        {
            tick();
        }
    }

    void Engine::terminate()
    {
        info("Teminate Engine.");
        g_context.destroy();
    }

    void Engine::tick()
    {
        logicalTick();
        renderTick();
    }

    void Engine::logicalTick() { g_context.m_window->pollEvents(); }

    void Engine::renderTick()
    {
        g_context.m_renderer->renderFrame();
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
