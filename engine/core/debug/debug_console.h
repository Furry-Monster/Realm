#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>

namespace RealmEngine
{
    enum class ConsoleLogLevel : uint8_t
    {
        trace,
        debug,
        info,
        warn,
        error,
        critical,
    };

    struct ConsoleLogEntry
    {
        ConsoleLogLevel level;
        std::string     message;
        uint64_t        timestamp_ms;
    };

    struct FrameStats
    {
        double frame_time_ms {0.0};
        double fps {0.0};
        int    draw_calls {0};
        int    triangle_count {0};
        size_t memory_rss_kb {0};
    };

    class EditorConsole
    {
    public:
        static constexpr size_t MAX_LOG_ENTRIES = 4096;

        static EditorConsole& instance();

        void pushLog(ConsoleLogLevel level, const std::string& message);
        void clearLogs();

        void setFrameStats(const FrameStats& stats)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_frame_stats = stats;
        }
        FrameStats getFrameStats() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_frame_stats;
        }

        void getLogs(std::deque<ConsoleLogEntry>& out, ConsoleLogLevel min_level) const;

    private:
        EditorConsole() = default;

        mutable std::mutex          m_mutex;
        std::deque<ConsoleLogEntry> m_logs;
        FrameStats                  m_frame_stats;
    };

} // namespace RealmEngine
