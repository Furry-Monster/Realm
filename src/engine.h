#pragma once

namespace RealmEngine
{
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
        void run();
        void terminate();

    protected:
        void tick();
        void logicalTick();
        void renderTick();
    };
} // namespace RealmEngine