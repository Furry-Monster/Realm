#include "global_context.h"

#include "logger.h"
#include "render/renderer.h"
#include "utils.h"
#include "window.h"

#include <memory>

namespace RealmEngine
{
    GlobalContext g_context;

    void GlobalContext::create()
    {
        m_logger = std::make_shared<Logger>();
        m_logger->initialize();

        m_window = std::make_shared<Window>();
        m_window->initialize();

        m_renderer = std::make_shared<Renderer>();
        m_renderer->initialize();
    }

    void GlobalContext::destroy()
    {
        m_renderer->disposal();
        m_renderer.reset();

        m_window->disposal();
        m_window.reset();

        m_logger->disposal();
        m_logger.reset();
    }
} // namespace RealmEngine