#include "logger.h"

#include <memory>
#include <string>
#include <string_view>

#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// NOTE: logger.cpp cannot include "utils.h" (would create circular dependency via global_context.h).
//       We use RE_PRETTY_FUNCTION directly and duplicate the extractClassFunction helper here.
#if defined(__GNUC__) || defined(__clang__)
#  define RE_PRETTY_FUNCTION_LOCAL __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define RE_PRETTY_FUNCTION_LOCAL __FUNCSIG__
#else
#  define RE_PRETTY_FUNCTION_LOCAL __func__
#endif

namespace RealmEngine
{
    namespace
    {
        inline std::string formatLogTag(const char* pretty_function)
        {
            std::string_view pf(pretty_function);
            auto             paren = pf.find('(');
            if (paren == std::string_view::npos)
                paren = pf.size();
            auto space     = pf.rfind(' ', paren);
            auto start     = (space == std::string_view::npos) ? std::string_view::size_type(0) : space + 1;
            auto qualified = pf.substr(start, paren - start);
            auto last_sep  = qualified.rfind("::");
            if (last_sep != std::string_view::npos)
            {
                auto before = (last_sep >= 2) ? qualified.rfind("::", last_sep - 2) : std::string_view::npos;
                if (before != std::string_view::npos)
                    qualified = qualified.substr(before + 2);
            }
            return "[" + std::string(qualified) + "] ";
        }
    } // namespace

    void Logger::initialize()
    {
        const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%^%l%$] %v");
        const spdlog::sinks_init_list init_list {console_sink};

        spdlog::init_thread_pool(8192, 1);
        m_spd_logger = std::make_shared<spdlog::async_logger>("realm_engine",
                                                              init_list.begin(),
                                                              init_list.end(),
                                                              spdlog::thread_pool(),
                                                              spdlog::async_overflow_policy::block);

        m_spd_logger->set_level(spdlog::level::trace);
        spdlog::register_logger(m_spd_logger);

        log(Logger::LogLevel::info, formatLogTag(RE_PRETTY_FUNCTION_LOCAL) + "Now tracing logs...");
    }

    void Logger::disposal()
    {
        log(Logger::LogLevel::info, formatLogTag(RE_PRETTY_FUNCTION_LOCAL) + "Stop logging and saving...");

        m_spd_logger->flush();
        spdlog::drop_all();
        m_spd_logger.reset();
    }

} // namespace RealmEngine
