#include "global_context.h"

#include "config_manager.h"
#include "input.h"
#include "logger.h"
#include "render/renderer.h"
#include "resource/asset_manager.h"
#include "window.h"

#include <memory>

namespace RealmEngine
{
    GlobalContext g_context;

    void GlobalContext::create()
    {
        m_logger = std::make_shared<Logger>();
        m_logger->initialize();

        m_config = std::make_shared<ConfigManager>();
        m_config->initialize();

        m_assets = std::make_shared<AssetManager>();
        m_assets->initialize();

        m_window = std::make_shared<Window>();
        m_window->initialize();

        m_renderer = std::make_shared<Renderer>();
        m_renderer->initialize(m_window);

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

        m_assets->disposal();
        m_assets.reset();

        m_config->disposal();
        m_config.reset();

        m_logger->disposal();
        m_logger.reset();
    }
} // namespace RealmEngine
