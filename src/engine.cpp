#include "engine.h"
#include "config_manager.h"
#include "global_context.h"
#include "render/renderer.h"
#include "resource/asset_manager.h"
#include "window.h"
#include <string>

namespace RealmEngine
{
    void Engine::boot()
    {
        g_context.create();
        // Model* model = g_context.m_assets->loadModel((g_context.m_cfg->getAssetFolder() /
        // "Cian.fbx").generic_string());
        info("<<< Boot Engine Done. >>>");
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
        info("<<< Now Teminating Engine. >>>");
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
