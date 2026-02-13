#include "global_context.h"

#include "core/event/event_bus.h"
#include "core/log/logger.h"
#include "platform/input/input.h"
#include "platform/window/window.h"
#include "renderer/renderer.h"
#include "resource/asset_manager.h"
#include "resource/config_manager.h"
#include "scene/scene_manager.h"

#include <memory>

namespace RealmEngine
{
    GlobalContext g_context;

    void GlobalContext::create()
    {
        // EventBus must be created first (other subsystems publish/subscribe)
        m_event_bus = std::make_shared<EventBus>();

        m_logger = std::make_shared<Logger>();
        m_logger->initialize();

        m_config = std::make_shared<ConfigManager>();
        m_config->initialize();

        m_assets = std::make_shared<AssetManager>();
        m_assets->initialize();

        m_scene = std::make_shared<SceneManager>();

        m_window = std::make_shared<Window>();
        m_window->initialize();

        m_renderer = std::make_shared<Renderer>();
        m_renderer->initialize();

        m_input = std::make_shared<Input>();
        m_input->initialize();
    }

    void GlobalContext::destroy()
    {
        m_input->disposal();
        m_input.reset();

        m_renderer->disposal();
        m_renderer.reset();

        m_window->disposal();
        m_window.reset();

        m_scene.reset();

        m_assets->disposal();
        m_assets.reset();

        m_config->disposal();
        m_config.reset();

        m_logger->disposal();
        m_logger.reset();

        // EventBus destroyed last
        m_event_bus.reset();
    }
} // namespace RealmEngine
