#pragma once

#include "widget.h"

#include "core/debug/debug_console.h"

#include <imgui.h>

namespace RealmEngine
{
    class ConsoleWidget : public Widget
    {
    public:
        ConsoleWidget();
        ~ConsoleWidget() override = default;

        ConsoleWidget(const ConsoleWidget&)            = delete;
        ConsoleWidget& operator=(const ConsoleWidget&) = delete;
        ConsoleWidget(ConsoleWidget&&)                 = delete;
        ConsoleWidget& operator=(ConsoleWidget&&)      = delete;

        void render() override;

    private:
        void        renderToolbar();
        void        renderLogList();
        const char* levelToString(ConsoleLogLevel level) const;
        ImVec4      levelToColor(ConsoleLogLevel level) const;

        ConsoleLogLevel             m_min_level {ConsoleLogLevel::trace};
        bool                        m_auto_scroll {true};
        bool                        m_show_timestamps {true};
        std::deque<ConsoleLogEntry> m_logs_cache;
    };

} // namespace RealmEngine
