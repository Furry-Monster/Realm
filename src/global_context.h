#pragma once

#include <memory>

namespace RealmEngine
{
    class Logger;
    class Window;

    class GlobalContext
    {
    public:
        GlobalContext()           = default;
        ~GlobalContext() noexcept = default;

        GlobalContext(const GlobalContext& that)            = delete;
        GlobalContext(GlobalContext&& that)                 = delete;
        GlobalContext& operator=(const GlobalContext& that) = delete;
        GlobalContext& operator=(GlobalContext&& that)      = delete;

        void create();
        void destroy();

        std::shared_ptr<Logger> m_logger;
        std::shared_ptr<Window> m_window;
    };

    extern GlobalContext g_context;
} // namespace RealmEngine