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

    protected:
        void tick();
        void logicalTick(std::shared_ptr<Scene> scene);
        void renderTick(std::shared_ptr<RenderScene> scene);

    private:
        std::shared_ptr<Scene>       m_scene;
        std::shared_ptr<RenderScene> m_render_scene;
    };
} // namespace RealmEngine