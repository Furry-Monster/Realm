#include "functional/ecs/scheduler.h"

#include <algorithm>

namespace RealmEngine
{
    void Scheduler::registerSystem(SystemPhase phase, std::string name, SystemFn fn)
    {
        if (m_systems.empty())
            m_systems.reserve(32);
        m_systems.push_back({phase, 0, std::move(name), std::move(fn)});
        m_dirty = true;
    }

    void Scheduler::registerSystem(SystemPhase phase, int order, std::string name, SystemFn fn)
    {
        if (m_systems.empty())
            m_systems.reserve(32);
        m_systems.push_back({phase, order, std::move(name), std::move(fn)});
        m_dirty = true;
    }

    bool Scheduler::unregisterSystem(const std::string& name)
    {
        auto it = std::remove_if(
            m_systems.begin(), m_systems.end(), [&name](const SystemEntry& e) { return e.name == name; });
        if (it == m_systems.end())
            return false;
        m_systems.erase(it, m_systems.end());
        m_dirty = true;
        return true;
    }

    void Scheduler::clear()
    {
        m_systems.clear();
        m_phase_ranges.fill({0, 0});
        m_dirty = false;
    }

    namespace
    {
        size_t phaseToIndex(SystemPhase p)
        {
            switch (p)
            {
                case SystemPhase::Logic:
                    return 0;
                case SystemPhase::PostLogic:
                    return 1;
                case SystemPhase::PreRender:
                    return 2;
                case SystemPhase::Render:
                    return 3;
                case SystemPhase::PostRender:
                    return 4;
                default:
                    return 5;
            }
        }
    } // namespace

    void Scheduler::ensureSorted()
    {
        if (!m_dirty)
            return;
        std::sort(m_systems.begin(), m_systems.end(), [](const SystemEntry& a, const SystemEntry& b) {
            if (a.phase != b.phase)
                return a.phase < b.phase;
            return a.order < b.order;
        });

        m_phase_ranges.fill({0, 0});

        for (size_t i = 0; i < m_systems.size();)
        {
            const SystemPhase p     = m_systems[i].phase;
            const size_t      idx   = phaseToIndex(p);
            const size_t      start = i;
            while (i < m_systems.size() && m_systems[i].phase == p)
                ++i;
            if (idx < m_phase_ranges.size())
                m_phase_ranges[idx] = {start, i - start};
        }

        m_dirty = false;
    }

    void Scheduler::prepare() { ensureSorted(); }

    void Scheduler::runPhase(SystemContext& ctx, SystemPhase phase)
    {
        ensureSorted();

        const size_t idx = phaseToIndex(phase);
        if (idx >= m_phase_ranges.size())
            return;

        const auto [start, count] = m_phase_ranges[idx];
        for (size_t i = 0; i < count; ++i)
            m_systems[start + i].fn(ctx);
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
