#include "functional/ecs/system_scheduler.h"

#include <algorithm>

namespace RealmEngine
{
    void SystemScheduler::registerSystem(SystemPhase phase, std::string name, SystemFn fn)
    {
        m_systems.push_back({phase, std::move(name), std::move(fn)});
    }

    void SystemScheduler::runPhase(SystemContext& ctx, SystemPhase phase)
    {
        for (auto& sys : m_systems)
        {
            if (sys.phase == phase)
                sys.fn(ctx);
        }
    }

    void SystemScheduler::tickLogical(SystemContext& ctx)
    {
        runPhase(ctx, SystemPhase::Logic);
        runPhase(ctx, SystemPhase::PostLogic);
    }

    void SystemScheduler::tickRender(SystemContext& ctx)
    {
        runPhase(ctx, SystemPhase::PreRender);
        runPhase(ctx, SystemPhase::Render);
        runPhase(ctx, SystemPhase::PostRender);
    }
} // namespace RealmEngine
