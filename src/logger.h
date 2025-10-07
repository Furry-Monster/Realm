#pragma once

#include <cstdint>
#include <memory>
#include <spdlog/logger.h>

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

        Logger();
        ~Logger() noexcept;

        Logger(const Logger& that)            = delete;
        Logger(Logger&& that)                 = delete;
        Logger& operator=(const Logger& that) = delete;
        Logger& operator=(Logger&& that)      = delete;

        template<typename... TARGS>
        void log(const LogLevel& level, TARGS&&... args) const
        {
            switch (level)
            {
                case LogLevel::debug:
                    m_logger->debug(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::info:
                    m_logger->info(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::warn:
                    m_logger->warn(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::error:
                    m_logger->error(std::forward<TARGS>(args)...);
                    break;
                case LogLevel::fatal:
                    m_logger->critical(std::forward<TARGS>(args)...);
                    const std::string fmt_str = fmt::format(std::forward<TARGS>(args)...);
                    throw std::runtime_error(fmt_str);
                    break;
            }
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };
} // namespace RealmEngine