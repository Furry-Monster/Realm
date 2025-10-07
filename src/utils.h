#include "global_context.h"
#include "logger.h"
#include <string>

namespace RealmEngine
{
    inline static void info(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::info, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void debug(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::debug, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void warn(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::warn, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void err(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::error, "[" + std::string(__FUNCTION__) + "]" + str);
    }

    inline static void fatal(std::string&& str)
    {
        g_context.m_logger->log(Logger::LogLevel::fatal, "[" + std::string(__FUNCTION__) + "]" + str);
    }

} // namespace RealmEngine