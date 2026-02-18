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
        ~ConsoleWidget() noexcept override = default;

        ConsoleWidget(const ConsoleWidget&)            = delete;
        ConsoleWidget& operator=(const ConsoleWidget&) = delete;
        ConsoleWidget(ConsoleWidget&&)                 = delete;
        ConsoleWidget& operator=(ConsoleWidget&&)      = delete;

        void render() override;

    private:
        void renderToolbar();
        void renderLogList();

        static const char* levelToString(ConsoleLogLevel level);
        static ImVec4      levelToColor(ConsoleLogLevel level);

        ConsoleLogLevel             m_min_level {ConsoleLogLevel::trace};
        bool                        m_auto_scroll {true};
        bool                        m_show_timestamps {true};
        std::deque<ConsoleLogEntry> m_logs_cache;
    };

} // namespace RealmEngine
