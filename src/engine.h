#pragma once

#include <memory>
#include "gameplay/scene.h"
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
        void debugRun();
        void run();
        void terminate();

        double m_delta_time {0.0f};

    protected:
        void tick();
        void logicalTick(std::shared_ptr<Scene> scene) const;
        void renderTick(std::shared_ptr<RenderScene> scene);

    private:
        // TODO: Scene and render scene shouldn't be directly managed by Engine.
        std::shared_ptr<Scene>       m_scene;
        std::shared_ptr<RenderScene> m_render_scene;

        double m_last_frame_time {0.0f};
    };
} // namespace RealmEngine
