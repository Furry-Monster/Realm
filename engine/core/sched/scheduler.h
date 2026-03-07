#pragma once

#include <array>
#include <functional>
#include <string>
#include <vector>

namespace RealmEngine
{
    class Scene;

    struct SystemContext
    {
        Scene* scene {nullptr};
        float  delta_time {0.0f};
        void*  engine {nullptr};
    };

    enum class SystemPhase : int
    {
        PreLogic   = -100,
        Logic      = 0,
        PostLogic  = 100,
        PreRender  = 200,
        Render     = 300,
        PostRender = 400,
    };

    using SystemFn = std::function<void(SystemContext&)>;

    struct SystemEntry
    {
        SystemPhase phase;
        int         order {0};
        std::string name;
        SystemFn    fn;
    };

    class Scheduler
    {
    public:
        void registerSystem(SystemPhase phase, std::string name, SystemFn fn);
        void registerSystem(SystemPhase phase, int order, std::string name, SystemFn fn);
        bool unregisterSystem(const std::string& name);
        void clear();

        void prepare();

        void tick(SystemContext& ctx);

    private:
        void ensureSorted();
        void runPhase(SystemContext& ctx, SystemPhase phase);

        std::vector<SystemEntry>                 m_systems;
        std::array<std::pair<size_t, size_t>, 6> m_phase_ranges {};
        bool                                     m_dirty {false};
    };
} // namespace RealmEngine
