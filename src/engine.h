#pragma once

#include <memory>
#include "gameplay/scene/scene.h"
#include "render/render_scene.h"

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

        double m_delta_time {0.0f};

    protected:
        void tick();
        void logicalTick(std::shared_ptr<Scene> scene) const;
        void renderTick();

    private:
        std::shared_ptr<Scene> createDefaultScene();

        std::shared_ptr<Scene> m_scene;

        double m_last_frame_time {0.0f};
    };
} // namespace RealmEngine
