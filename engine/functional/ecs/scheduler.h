#pragma once

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
        Logic      = 0,
        PostLogic  = 100,
        PreRender  = 200,
        Render     = 300,
        PostRender = 400,
    };

    using SystemFn = std::function<void(SystemContext&)>;

    struct RegisteredSystem
    {
        SystemPhase phase;
        std::string name;
        SystemFn    fn;
    };

    class Scheduler
    {
    public:
        void registerSystem(SystemPhase phase, std::string name, SystemFn fn);
        void tickLogical(SystemContext& ctx);
        void tickRender(SystemContext& ctx);

    private:
        void runPhase(SystemContext& ctx, SystemPhase phase);

        std::vector<RegisteredSystem> m_systems;
    };
} // namespace RealmEngine
