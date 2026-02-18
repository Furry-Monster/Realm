#pragma once

#include "core/debug/debug_console.h"

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

#include <mutex>

namespace RealmEngine
{
    inline ConsoleLogLevel spdlogLevelToConsole(const spdlog::level::level_enum level)
    {
        switch (level)
        {
            case spdlog::level::trace:
                return ConsoleLogLevel::trace;
            case spdlog::level::debug:
                return ConsoleLogLevel::debug;
            case spdlog::level::info:
                return ConsoleLogLevel::info;
            case spdlog::level::warn:
                return ConsoleLogLevel::warn;
            case spdlog::level::err:
                return ConsoleLogLevel::error;
            case spdlog::level::critical:
                return ConsoleLogLevel::critical;
            default:
                return ConsoleLogLevel::info;
        }
    }

    class ConsoleSink : public spdlog::sinks::base_sink<std::mutex>
    {
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            base_sink<std::mutex>::formatter_->format(msg, formatted);
            std::string str(formatted.data(), formatted.size());
            while (!str.empty() && (str.back() == '\n' || str.back() == '\r'))
                str.pop_back();
            EditorConsole::instance().pushLog(spdlogLevelToConsole(msg.level), str);
        }
        void flush_() override {}
    };

} // namespace RealmEngine
