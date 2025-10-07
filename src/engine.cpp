#include "engine.h"
#include "global_context.h"
#include "utils.h"

namespace RealmEngine
{
    void Engine::boot()
    {
        g_context.create();
        info("Boot Engine.");
    }

    void Engine::run()
    {
        while (true)
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

    void Engine::logicalTick() {}

    void Engine::renderTick() {}

} // namespace RealmEngine
