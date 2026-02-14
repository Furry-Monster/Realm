#include "core/debug/debug_console.h"

#include <chrono>

namespace RealmEngine
{
    EditorConsole& EditorConsole::instance()
    {
        static EditorConsole s_instance;
        return s_instance;
    }

    void EditorConsole::pushLog(ConsoleLogLevel level, const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t                    now = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count());
        m_logs.emplace_back(ConsoleLogEntry {level, message, now});
        while (m_logs.size() > MAX_LOG_ENTRIES)
            m_logs.pop_front();
    }

    void EditorConsole::clearLogs()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logs.clear();
    }

    void EditorConsole::getLogs(std::deque<ConsoleLogEntry>& out, ConsoleLogLevel min_level) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        out.clear();
        for (const auto& entry : m_logs)
        {
            if (entry.level >= min_level)
                out.push_back(entry);
        }
    }

} // namespace RealmEngine
