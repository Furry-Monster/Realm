#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>

#include "core/base/macros.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "core/log/logger.h"
#include "core/sched/scheduler.h"
#include "functional/render/renderer.h"
#include "functional/render/viewport_controller.h"
#include "functional/resource/asset_manager.h"
#include "functional/resource/config_manager.h"
#include "functional/scene/scene.h"
#include "functional/scene/scene_manager.h"
#include "module_manager.h"
#include "platform/info/platform_info.h"
#include "platform/input/input.h"
#include "platform/window/window.h"
#include "registration.h"

namespace RealmEngine
{
    Engine::Engine()           = default;
    Engine::~Engine() noexcept = default;

    EventBus& Engine::getEventBus() const
    {
        assert(m_event_bus && "Engine not initialized");
        return *m_event_bus;
    }
    Logger& Engine::getLogger() const
    {
        assert(m_logger && "Engine not initialized");
        return *m_logger;
    }
    ConfigManager& Engine::getConfig() const
    {
        assert(m_config && "Engine not initialized");
        return *m_config;
    }
    AssetManager& Engine::getAssets() const
    {
        assert(m_assets && "Engine not initialized");
        return *m_assets;
    }
    SceneManager& Engine::getSceneManager() const
    {
        assert(m_scene && "Engine not initialized");
        return *m_scene;
    }
    Window& Engine::getWindow() const
    {
        assert(m_window && "Engine not initialized");
        return *m_window;
    }
    Renderer& Engine::getRenderer() const
    {
        assert(m_renderer && "Engine not initialized");
        return *m_renderer;
    }
    Input& Engine::getInput() const
    {
        assert(m_input && "Engine not initialized");
        return *m_input;
    }
    ModuleManager& Engine::getModuleManager()
    {
        assert(m_modules && "Engine not initialized");
        return *m_modules;
    }
    const ModuleManager& Engine::getModuleManager() const
    {
        assert(m_modules && "Engine not initialized");
        return *m_modules;
    }
    Scheduler& Engine::getSystemScheduler()
    {
        assert(m_scheduler && "Engine not initialized");
        return *m_scheduler;
    }
    const Scheduler& Engine::getSystemScheduler() const
    {
        assert(m_scheduler && "Engine not initialized");
        return *m_scheduler;
    }

    void Engine::initialize()
    {
        try
        {
            m_event_bus = std::make_unique<EventBus>();
            g_event_bus = m_event_bus.get();

            m_logger = std::make_unique<Logger>();
            m_logger->initialize();
            g_logger = m_logger.get();

            m_config = std::make_unique<ConfigManager>();
            m_config->initialize();

            m_assets = std::make_unique<AssetManager>();
            m_assets->initialize();

            m_scene = std::make_unique<SceneManager>();
            m_scene->initialize(m_config->getAssetFolder());
            m_scene->setAssetManager(m_assets.get());
            m_scene->setOnSceneChanged(
                [this](const std::shared_ptr<Scene>& old_scene, const std::shared_ptr<Scene>& new_scene) {
                    m_event_bus->publish(SceneChangedEvent {old_scene.get(), new_scene.get()});
                });

            m_window = std::make_unique<Window>();
            m_window->initialize(m_config->getWindowConfig());

            m_renderer = std::make_unique<Renderer>();
            m_renderer->initialize(*m_config, *m_window);

            // Forward framebuffer resize events to the renderer (Engine outlives EventBus)
            (void)m_event_bus->subscribe<FramebufferResizeEvent>(
                [this](const FramebufferResizeEvent& e) { m_renderer->onResize(e.width, e.height); });

            m_input = std::make_unique<Input>();
            m_input->initialize(*m_window);

            m_modules = std::make_unique<ModuleManager>();
            m_modules->initialize(*m_config, *m_event_bus);

            m_scheduler = std::make_unique<Scheduler>();
            registerSystems(*m_scheduler, *this);
            m_scheduler->prepare();

            const EngineConfig& engine_config = m_config->getEngineConfig();
            m_max_delta_time                  = engine_config.max_delta_time;

            m_last_frame_time = m_window->getTime();

            PlatformInfo::logPlatformInfo(m_renderer->getDevice());

            m_initialized = true;
            RE_LOG_INFO("<<< Boot Engine Done. >>>");
        }
        catch (...)
        {
            g_event_bus = nullptr;
            g_logger    = nullptr;

            m_scheduler.reset();
            m_modules.reset();
            m_input.reset();
            m_renderer.reset();
            m_window.reset();
            m_scene.reset();
            m_assets.reset();
            m_config.reset();
            m_logger.reset();
            m_event_bus.reset();
            throw;
        }
    }

    void Engine::shutdown()
    {
        RE_LOG_INFO("<<< Now Terminating Engine. >>>");
        if (!m_initialized)
            return;

        m_initialized = false;
        m_delta_time  = 0.0;

        m_modules->shutdown();
        m_modules.reset();

        m_input->disposal();
        m_input.reset();

        m_renderer->disposal();
        m_renderer.reset();

        m_scene.reset();

        m_assets->disposal();
        m_assets.reset();

        m_window->disposal();
        m_window.reset();

        m_config->disposal();
        m_config.reset();

        g_logger = nullptr;
        m_logger->disposal();
        m_logger.reset();

        g_event_bus = nullptr;
        m_event_bus.reset();
    }

    void Engine::loop()
    {
        const std::filesystem::path scene_file = m_config->getRootFolder() / m_config->getEngineConfig().scene_file;

        RHIDevice& device = m_renderer->getDevice();

        std::shared_ptr<Scene> loaded;
        if (std::filesystem::exists(scene_file))
        {
            RE_LOG_INFO("Loading scene from: " + scene_file.string());
            loaded = m_scene->loadScene(scene_file.string(), device);
        }

        if (!loaded)
        {
            RE_LOG_INFO("Loading failed, create default scene instead.");
            loaded = m_scene->createDefaultScene(device);
        }

        m_scene->setCurrentScene(loaded);
        setViewportMode(ViewportMode::Game);

        loaded->setViewportController(std::make_shared<ViewportController>());
        const ViewportConfig& v = m_config->getViewportConfig();
        loaded->getViewportController()->initialize(m_renderer->getCamera(),
                                                    *m_input,
                                                    v.camera_mouse_sensitivity,
                                                    v.camera_move_speed,
                                                    v.camera_sprint_multiplier);

        const RendererConfig& rc = m_config->getRendererConfig();
        m_renderer->getCamera()->setPosition(
            glm::vec3(rc.camera_initial_pos_x, rc.camera_initial_pos_y, rc.camera_initial_pos_z));
        m_renderer->getCamera()->lookAt(glm::vec3(rc.camera_look_at_x, rc.camera_look_at_y, rc.camera_look_at_z));

        RE_LOG_INFO("<<< Run in Debug-Mode. >>>");

        while (!m_window->shouldClose())
        {
            tick();
            m_window->swapBuffer();
        }
    }

    void Engine::tick()
    {
        const double current_time = m_window->getTime();
        m_delta_time              = current_time - m_last_frame_time;
        m_last_frame_time         = current_time;
        if (m_delta_time > m_max_delta_time)
            m_delta_time = m_max_delta_time;

        logicalTick();
        renderTick();
    }

    void Engine::logicalTick()
    {
        m_input->tick();
        m_window->pollEvents();

        auto* scene = m_scene->getCurrentOrNewScene().get();
        if (scene && !scene->getViewportController())
            scene->setViewportController(std::make_shared<ViewportController>());

        SystemContext ctx {};
        ctx.scene      = scene;
        ctx.delta_time = static_cast<float>(m_delta_time);
        ctx.engine     = this;
        m_scheduler->tickLogical(ctx);
    }

    void Engine::renderTick()
    {
        auto* scene = m_scene->getCurrentScene().get();

        SystemContext ctx {};
        ctx.scene      = scene;
        ctx.delta_time = static_cast<float>(m_delta_time);
        ctx.engine     = this;
        m_scheduler->tickRender(ctx);
    }

} // namespace RealmEngine
