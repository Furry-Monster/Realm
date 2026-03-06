#include "functional/ecs/scheduler.h"

#include <algorithm>

namespace RealmEngine
{
    void Scheduler::registerSystem(SystemPhase phase, std::string name, SystemFn fn)
    {
        m_systems.push_back({phase, std::move(name), std::move(fn)});
    }

    void Scheduler::runPhase(SystemContext& ctx, SystemPhase phase)
    {
        for (auto& sys : m_systems)
        {
            if (sys.phase == phase)
                sys.fn(ctx);
        }
    }

    void Scheduler::tickLogical(SystemContext& ctx)
    {
        runPhase(ctx, SystemPhase::Logic);
        runPhase(ctx, SystemPhase::PostLogic);
    }

    void Scheduler::tickRender(SystemContext& ctx)
    {
        runPhase(ctx, SystemPhase::PreRender);
        runPhase(ctx, SystemPhase::Render);
        runPhase(ctx, SystemPhase::PostRender);
    }
} // namespace RealmEngine
