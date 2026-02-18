#pragma once

#include <spdlog/logger.h>
#include <memory>

namespace RealmEngine
{
    class Logger final
    {
    public:
        enum class LogLevel : uint8_t
        {
            debug,
            info,
            warn,
            error,
            fatal,
        };

        Logger()           = default;
        ~Logger() noexcept = default;

        Logger(const Logger& that)                = delete;
        Logger& operator=(const Logger& that)     = delete;
        Logger(Logger&& that) noexcept            = default;
        Logger& operator=(Logger&& that) noexcept = default;

        void initialize();
        void disposal();

        template<typename... TARGS>
        void log(const LogLevel& level, TARGS&&... args) const
        {
            switch (level)
            {
                case LogLevel::debug:
                    m_spd_logger->debug(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::info:
                    m_spd_logger->info(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::warn:
                    m_spd_logger->warn(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::error:
                    m_spd_logger->error(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::fatal: {
                    const std::string fmt_str = fmt::format(std::forward<TARGS>(args)...);
                    m_spd_logger->critical(fmt_str);
                    throw std::runtime_error(fmt_str);
                }
            }
        }

    private:
        std::shared_ptr<spdlog::logger> m_spd_logger;
    };

    // NOTE: global non-owning logger pointer, set by Engine during boot
    extern Logger* g_logger;

} // namespace RealmEngine
