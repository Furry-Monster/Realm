#pragma once

#include <memory>

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

    class Engine
    {
    public:
        Engine();
        ~Engine() noexcept;

        Engine(const Engine&)            = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&)      = delete;

        void boot();
        void shutdown();

        // Runtime stand-alone loop (loads scene, runs until window close)
        void debug();

        void tick();

        double getDeltaTime() const { return m_delta_time; }

        // Subsystem accessors
        EventBus&      getEventBus();
        Logger&        getLogger();
        ConfigManager& getConfig();
        AssetManager&  getAssets();
        SceneManager&  getSceneManager();
        Window&        getWindow();
        Renderer&      getRenderer();
        Input&         getInput();

    protected:
        void logicalTick();
        void renderTick();

    private:
        // Subsystems (order matters for init/destroy)
        std::unique_ptr<EventBus>      m_event_bus;
        std::unique_ptr<Logger>        m_logger;
        std::unique_ptr<ConfigManager> m_config;
        std::unique_ptr<AssetManager>  m_assets;
        std::unique_ptr<SceneManager>  m_scene;
        std::unique_ptr<Window>        m_window;
        std::unique_ptr<Renderer>      m_renderer;
        std::unique_ptr<Input>         m_input;

        double m_delta_time {0.0};
        double m_max_delta_time {0.1};
        double m_last_frame_time {0.0};
    };
} // namespace RealmEngine
