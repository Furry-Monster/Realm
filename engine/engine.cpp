#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <filesystem>
#include <memory>
#include <string>

#include "core/base/macros.h"
#include "core/debug/debug_console.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "core/log/logger.h"
#include "platform/info/platform_info.h"
#include "platform/input/input.h"
#include "platform/window/window.h"
#include "render/light_probe_baker.h"
#include "render/renderer.h"
#include "resource/asset_manager.h"
#include "resource/config_manager.h"
#include "scene/components/camera_controller.h"
#include "scene/components/lighting/light_probe.h"
#include "scene/components/transform.h"
#include "scene/components/world_transform.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"

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
            m_window->initialize(*m_event_bus, m_config->getWindowConfig());

            m_renderer = std::make_unique<Renderer>();
            m_renderer->initialize(*m_config, *m_window);

            // Forward framebuffer resize events to the renderer (Engine outlives EventBus)
            (void)m_event_bus->subscribe<FramebufferResizeEvent>(
                [this](const FramebufferResizeEvent& e) { m_renderer->onResize(e.width, e.height); });

            m_input = std::make_unique<Input>();
            m_input->initialize(*m_event_bus, *m_window);

            const GamePlayConfig& gameplay_config = m_config->getGamePlayConfig();
            m_max_delta_time                      = gameplay_config.max_delta_time;

            m_last_frame_time = m_window->getTime();

            PlatformInfo::logPlatformInfo(m_renderer->getDevice());

            m_initialized = true;
            RE_LOG_INFO("<<< Boot Engine Done. >>>");
        }
        catch (...)
        {
            // Ensure clean state on partial initialization failure
            g_logger = nullptr;
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

        m_initialized = false;
        m_delta_time  = 0.0;

        // Shutdown in reverse initialization order
        m_input->disposal(*m_event_bus);
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

        // Clear global logger before destroying it
        g_logger = nullptr;
        m_logger->disposal();
        m_logger.reset();

        // EventBus last
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

        // Initialize scene camera controller
        const GamePlayConfig& gp = m_config->getGamePlayConfig();
        loaded->getCameraController()->initialize(m_renderer->getCamera(),
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

    void Engine::logicalTick()
    {
        m_input->tick();
        m_window->pollEvents();
        m_scene->getCurrentOrNewScene()->tick(static_cast<float>(m_delta_time));
    }

    void Engine::renderTick()
    {
        const auto scene = m_scene->getCurrentScene();
        m_renderer->getRenderScene()->syncFromScene(scene);

        if (scene && m_renderer->getLightProbeBaker())
        {
            auto&      registry = scene->getRegistry();
            const auto view     = registry.view<LightProbe>();

            for (const auto entity : view)
            {
                auto& lp = view.get<LightProbe>(entity);
                if (!lp.needs_update)
                    continue;

                glm::vec3 pos {0.0f};
                if (auto* wt = scene->tryGet<WorldTransform>(entity))
                    pos = glm::vec3(wt->matrix[3]);
                else if (const auto* t = scene->tryGet<Transform>(entity))
                    pos = t->position;

                const auto [sh_coefficients, success] =
                    m_renderer->getLightProbeBaker()->bake(pos, *m_renderer->getRenderScene());
                if (success)
                {
                    lp.sh_coefficients = sh_coefficients;
                    lp.needs_update    = false;
                }
            }
        }

        m_renderer->render();

        FrameStats stats {};
        stats.frame_time_ms  = m_delta_time * 1000.0;
        stats.fps            = (m_delta_time > 1e-9) ? (1.0 / m_delta_time) : 0.0;
        stats.draw_calls     = m_renderer->getRenderScene()->getDrawCallCount();
        stats.triangle_count = m_renderer->getRenderScene()->getTriangleCount();

        // Throttle RSS syscall to ~1Hz
        static double s_rss_accumulator = 0.0;
        static size_t s_cached_rss      = 0;
        s_rss_accumulator += m_delta_time;
        if (s_rss_accumulator >= 1.0)
        {
            s_cached_rss      = PlatformInfo::getProcessRSSKB();
            s_rss_accumulator = 0.0;
        }
        stats.memory_rss_kb = s_cached_rss;

        EditorConsole::instance().setFrameStats(stats);
    }

} // namespace RealmEngine
