#include "logger.h"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace RealmEngine
{
    void Logger::initialize()
    {
        const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%^%l%$] %v");
        const spdlog::sinks_init_list init_list {console_sink};

        spdlog::init_thread_pool(8192, 1);
        m_spd_logger = std::make_shared<spdlog::async_logger>("muggle_logger",
                                                              init_list.begin(),
                                                              init_list.end(),
                                                              spdlog::thread_pool(),
                                                              spdlog::async_overflow_policy::block);

        m_spd_logger->set_level(spdlog::level::trace);
        spdlog::register_logger(m_spd_logger);
    }

    void Logger::disposal()
    {
        m_spd_logger->flush();
        spdlog::drop_all();
        m_spd_logger.reset();
    }

} // namespace RealmEngine
