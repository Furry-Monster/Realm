#include "global_context.h"

#include "config_manager.h"
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

        m_cfg = std::make_shared<ConfigManager>();
        m_cfg->initialize();

        m_assets = std::make_shared<AssetManager>();
        m_assets->initialize();

        m_window = std::make_shared<Window>();
        m_window->initialize();

        m_renderer = std::make_shared<Renderer>();
        m_renderer->initialize();
        m_renderer->setPipeline(PipelineType::Forward);
    }

    void GlobalContext::destroy()
    {
        m_renderer->disposal();
        m_renderer.reset();

        m_window->disposal();
        m_window.reset();

        m_assets->disposal();
        m_assets.reset();

        m_cfg->disposal();
        m_cfg.reset();

        m_logger->disposal();
        m_logger.reset();
    }
} // namespace RealmEngine