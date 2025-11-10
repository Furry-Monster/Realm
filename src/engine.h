#pragma once

#include <memory>

namespace RealmEngine
{
    class Scene;

    class Engine
    {
    public:
        Engine()           = default;
        ~Engine() noexcept = default;

        Engine(const Engine& that)            = delete;
        Engine(Engine&& that)                 = delete;
        Engine& operator=(const Engine& that) = delete;
        Engine& operator=(Engine&& that)      = delete;

        void boot();
        void debugRun();
        void run();
        void terminate();

    protected:
        void tick(std::shared_ptr<Scene> scene);
        void logicalTick();
        void renderTick(std::shared_ptr<Scene> scene);
    };
} // namespace RealmEngine