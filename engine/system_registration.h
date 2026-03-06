#pragma once

namespace RealmEngine
{
    class Engine;
    class SystemScheduler;

    void registerEngineSystems(SystemScheduler& scheduler, Engine& engine);
} // namespace RealmEngine
