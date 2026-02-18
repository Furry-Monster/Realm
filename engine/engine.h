#pragma once

#include <cassert>
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

        void initialize();
        void shutdown();

        void loop();
        void tick();

        bool   isInitialized() const { return m_initialized; }
        double getDeltaTime() const { return m_delta_time; }

        // Subsystem accessors (assert that the engine has been initialized)
        EventBus& getEventBus() const
        {
            assert(m_event_bus && "Engine not initialized");
            return *m_event_bus;
        }
        Logger& getLogger() const
        {
            assert(m_logger && "Engine not initialized");
            return *m_logger;
        }
        ConfigManager& getConfig() const
        {
            assert(m_config && "Engine not initialized");
            return *m_config;
        }
        AssetManager& getAssets() const
        {
            assert(m_assets && "Engine not initialized");
            return *m_assets;
        }
        SceneManager& getSceneManager() const
        {
            assert(m_scene && "Engine not initialized");
            return *m_scene;
        }
        Window& getWindow() const
        {
            assert(m_window && "Engine not initialized");
            return *m_window;
        }
        Renderer& getRenderer() const
        {
            assert(m_renderer && "Engine not initialized");
            return *m_renderer;
        }
        Input& getInput() const
        {
            assert(m_input && "Engine not initialized");
            return *m_input;
        }

    protected:
        void logicalTick();
        void renderTick();

    private:
        // Subsystems (declaration order = destruction order when using reset())
        std::unique_ptr<EventBus>      m_event_bus;
        std::unique_ptr<Logger>        m_logger;
        std::unique_ptr<ConfigManager> m_config;
        std::unique_ptr<AssetManager>  m_assets;
        std::unique_ptr<SceneManager>  m_scene;
        std::unique_ptr<Window>        m_window;
        std::unique_ptr<Renderer>      m_renderer;
        std::unique_ptr<Input>         m_input;

        bool   m_initialized {false};
        double m_delta_time {0.0};
        double m_max_delta_time {0.1};
        double m_last_frame_time {0.0};
    };
} // namespace RealmEngine
