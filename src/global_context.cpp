#include "global_context.h"
#include "logger.h"

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
    }

    void GlobalContext::destroy()
    {
        m_window->disposal();
        m_window.reset();

        m_logger->disposal();
        m_logger.reset();
    }
} // namespace RealmEngine