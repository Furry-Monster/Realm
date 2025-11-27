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
        void debug();
        void tick();
        void terminate();

    protected:
        void logicalTick() const;
        void renderTick() const;

    private:
        double m_delta_time {0.0f};
        double m_max_delta_time {0.1f};
        double m_last_frame_time {0.0f};
    };
} // namespace RealmEngine
