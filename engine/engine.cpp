#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <filesystem>
#include <memory>
#include <string>

#include "core/base/macros.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "core/log/logger.h"
#include "functional/ecs/system_scheduler.h"
#include "functional/resource/asset_manager.h"
#include "functional/resource/config_manager.h"
#include "functional/scene/scene.h"
#include "functional/scene/scene_manager.h"
#include "module/audio/audio_system.h"
#include "module/render/renderer.h"
#include "module/render/viewport_controller.h"
#include "platform/info/platform_info.h"
#include "platform/input/input.h"
#include "platform/window/window.h"
#include "system_registration.h"

namespace RealmEngine
{
    Engine::Engine()           = default;
    Engine::~Engine() noexcept = default;

    void Engine::initialize()
    {
        try
        {
            // EventBus first (other subsystems publish/subscribe)
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

            (void)m_event_bus->subscribe<SceneChangedEvent>([this](const SceneChangedEvent&) {
                if (m_audio)
                    m_audio->clearSceneSounds();
            });

            m_input = std::make_unique<Input>();
            m_input->initialize(*m_window);

            m_audio = std::make_unique<AudioSystem>();
            m_audio->initialize(m_config->getAudioConfig());

            m_scheduler = std::make_unique<SystemScheduler>();
            registerEngineSystems(*m_scheduler, *this);

            const GamePlayConfig& gameplay_config = m_config->getGamePlayConfig();
            m_max_delta_time                      = gameplay_config.max_delta_time;

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
            m_audio.reset();
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

        if (m_audio)
        {
            m_audio->shutdown();
            m_audio.reset();
        }
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

        g_logger    = nullptr;
        g_event_bus = nullptr;
        m_logger->disposal();
        m_logger.reset();

        m_event_bus.reset();
    }

    void Engine::loop()
    {
        const std::filesystem::path scene_file = m_config->getRootFolder() / m_config->getGamePlayConfig().scene_file;

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
        const GamePlayConfig& gp = m_config->getGamePlayConfig();
        loaded->getViewportController()->initialize(m_renderer->getCamera(),
                                                    *m_input,
                                                    gp.camera_mouse_sensitivity,
                                                    gp.camera_move_speed,
                                                    gp.camera_sprint_multiplier);

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

    SystemScheduler& Engine::getSystemScheduler()
    {
        assert(m_scheduler && "Engine not initialized");
        return *m_scheduler;
    }

    const SystemScheduler& Engine::getSystemScheduler() const
    {
        assert(m_scheduler && "Engine not initialized");
        return *m_scheduler;
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
