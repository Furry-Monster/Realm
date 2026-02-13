#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <filesystem>
#include <memory>
#include <string>

#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "core/log/log_macros.h"
#include "core/log/logger.h"
#include "platform/info/platform_info.h"
#include "platform/input/input.h"
#include "platform/window/window.h"
#include "renderer/renderer.h"
#include "resource/asset_manager.h"
#include "resource/config_manager.h"
#include "rhi/rhi_device.h"
#include "scene/components/camera_controller.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"

namespace RealmEngine
{
    Engine::Engine()           = default;
    Engine::~Engine() noexcept = default;

    void Engine::boot()
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

        m_window = std::make_unique<Window>();
        m_window->initialize(*m_event_bus, m_config->getWindowConfig());

        m_renderer = std::make_unique<Renderer>();
        m_renderer->initialize(*m_config, *m_window);

        // Forward framebuffer resize events to the renderer
        m_event_bus->subscribe<FramebufferResizeEvent>(
            [this](const FramebufferResizeEvent& e) { m_renderer->onResize(e.width, e.height); });

        m_input = std::make_unique<Input>();
        m_input->initialize(*m_event_bus, *m_window);

        const GamePlayConfig& gameplay_config = m_config->getGamePlayConfig();
        m_max_delta_time                      = gameplay_config.max_delta_time;

        m_last_frame_time = m_window->getTime();

        PlatformInfo::logPlatformInfo();

        RE_LOG_INFO("<<< Boot Engine Done. >>>");
    }

    void Engine::shutdown()
    {
        RE_LOG_INFO("<<< Now Terminating Engine. >>>");

        m_delta_time = 0.0;

        m_input->disposal(*m_event_bus);
        m_input.reset();

        m_scene.reset();

        m_renderer->disposal();
        m_renderer.reset();

        m_window->disposal();
        m_window.reset();

        m_assets->disposal();
        m_assets.reset();

        m_config->disposal();
        m_config.reset();

        m_logger->disposal();
        g_logger = nullptr;
        m_logger.reset();

        // EventBus last
        m_event_bus.reset();
    }

    void Engine::debug()
    {
        std::filesystem::path scene_file = m_config->getRootFolder() / m_config->getGamePlayConfig().scene_file;

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

        m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        RE_LOG_INFO("<<< Run in Debug-Mode. >>>");

        while (!m_window->shouldClose())
        {
            tick();
            m_window->swapBuffer();
        }

        if (m_scene->getCurrentScene())
        {
            std::filesystem::path save_path = m_config->getRootFolder() / m_config->getGamePlayConfig().scene_file;

            RE_LOG_INFO("Saving scene to: " + save_path.string());
            if (m_scene->saveCurrentScene(save_path.string()))
                RE_LOG_INFO("Scene saved successfully.");
            else
                RE_LOG_ERROR("Failed to save scene file.");
        }
    }

    void Engine::tick()
    {
        double current_time = m_window->getTime();
        m_delta_time        = current_time - m_last_frame_time;
        m_last_frame_time   = current_time;
        if (m_delta_time > m_max_delta_time)
            m_delta_time = m_max_delta_time;

        logicalTick();
        renderTick();
    }

    void Engine::logicalTick()
    {
        m_input->tick();
        m_window->pollEvents();
        m_scene->getCurrentOrNewScene()->tick(m_delta_time);
    }

    void Engine::renderTick()
    {
        m_renderer->getRenderScene()->syncFromScene(m_scene->getCurrentScene());
        m_renderer->render();
    }

    // Subsystem accessors
    EventBus&      Engine::getEventBus() { return *m_event_bus; }
    Logger&        Engine::getLogger() { return *m_logger; }
    ConfigManager& Engine::getConfig() { return *m_config; }
    AssetManager&  Engine::getAssets() { return *m_assets; }
    SceneManager&  Engine::getSceneManager() { return *m_scene; }
    Window&        Engine::getWindow() { return *m_window; }
    Renderer&      Engine::getRenderer() { return *m_renderer; }
    Input&         Engine::getInput() { return *m_input; }

} // namespace RealmEngine
