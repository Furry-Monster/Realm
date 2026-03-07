#pragma once

#include <memory>

#include "functional/render/viewport_display_mode.h"
#include "module_manager.h"

namespace RealmEngine
{
    class EventBus;
    class Logger;
    class ConfigManager;
    class AssetManager;
    class SceneManager;
    class Window;
    class Renderer;
    class Input;
    class Scene;
    class Scheduler;

    class Engine
    {
    public:
        Engine();
        ~Engine() noexcept;

        Engine(const Engine&)            = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&)      = delete;

        void initialize();
        void shutdown();

        void loop();
        void tick();

        bool   isInitialized() const { return m_initialized; }
        double getDeltaTime() const { return m_delta_time; }

        EventBus&      getEventBus() const;
        Logger&        getLogger() const;
        ConfigManager& getConfig() const;
        AssetManager&  getAssets() const;
        SceneManager&  getSceneManager() const;
        Window&        getWindow() const;
        Renderer&      getRenderer() const;
        Input&         getInput() const;

        ModuleManager&       getModuleManager();
        const ModuleManager& getModuleManager() const;
        Scheduler&           getSystemScheduler();
        const Scheduler&     getSystemScheduler() const;

        ViewportMode getViewportMode() const { return m_viewport_mode; }
        void         setViewportMode(ViewportMode mode) { m_viewport_mode = mode; }

    private:
        // below are engine-managed but globally accessible.
        std::unique_ptr<EventBus> m_event_bus;
        std::unique_ptr<Logger>   m_logger;

        std::unique_ptr<ConfigManager> m_config;
        std::unique_ptr<AssetManager>  m_assets;
        std::unique_ptr<SceneManager>  m_scene;
        std::unique_ptr<Window>        m_window;
        std::unique_ptr<Renderer>      m_renderer;
        std::unique_ptr<Input>         m_input;

        std::unique_ptr<ModuleManager> m_modules;
        std::unique_ptr<Scheduler>     m_scheduler;

        bool         m_initialized {false};
        double       m_delta_time {0.0};
        double       m_max_delta_time {0.1};
        double       m_last_frame_time {0.0};
        ViewportMode m_viewport_mode {ViewportMode::Scene};
    };
} // namespace RealmEngine
