#pragma once

#include <memory>

namespace RealmEngine
{
    class Logger;
    class ConfigManager;
    class AssetManager;
    class Window;
    class Renderer;
    class Input;

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

        std::shared_ptr<Logger>        m_logger;
        std::shared_ptr<ConfigManager> m_config;
        std::shared_ptr<AssetManager>  m_assets;
        std::shared_ptr<Window>        m_window;
        std::shared_ptr<Renderer>      m_renderer;
        std::shared_ptr<Input>         m_input;
    };

    extern GlobalContext g_context;

} // namespace RealmEngine
