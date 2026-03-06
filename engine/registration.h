#pragma once

namespace RealmEngine
{
    class Engine;
    class SystemScheduler;

    void registerSystems(SystemScheduler& scheduler, Engine& engine);
} // namespace RealmEngine
