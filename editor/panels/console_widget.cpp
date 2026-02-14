#include "panels/console_widget.h"

#include "core/debug/debug_console.h"

#include <imgui.h>
#include <cstdint>
#include <cstdio>

namespace RealmEngine
{
    ConsoleWidget::ConsoleWidget() : Widget("Console") {}

    void ConsoleWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_name.c_str(), &m_open))
        {
            ImGui::End();
            return;
        }

        renderToolbar();
        renderLogList();

        ImGui::End();
    }

    void ConsoleWidget::renderToolbar()
    {
        if (ImGui::Button("Clear"))
            EditorConsole::instance().clearLogs();

        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_auto_scroll);

        ImGui::SameLine();
        ImGui::Checkbox("Timestamps", &m_show_timestamps);

        ImGui::SameLine();
        ImGui::Text("Filter:");
        ImGui::SameLine();
        const char* levels[] = {"Trace", "Debug", "Info", "Warn", "Error", "Critical"};
        int         current  = static_cast<int>(m_min_level);
        if (ImGui::Combo("##level", &current, levels, 6))
            m_min_level = static_cast<ConsoleLogLevel>(current);

        ImGui::Separator();
    }

    void ConsoleWidget::renderLogList()
    {
        EditorConsole::instance().getLogs(m_logs_cache, m_min_level);

        ImGui::BeginChild("LogList", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& entry : m_logs_cache)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, levelToColor(entry.level));
            if (m_show_timestamps)
            {
                uint64_t s   = entry.timestamp_ms / 1000;
                uint64_t h   = s / 3600;
                uint64_t m   = (s % 3600) / 60;
                uint64_t sec = s % 60;
                uint64_t ms  = entry.timestamp_ms % 1000;
                char     buf[32];
                snprintf(buf, sizeof(buf), "[%02lu:%02lu:%02lu.%03lu] ", h, m, sec, ms);
                ImGui::TextUnformatted(buf);
                ImGui::SameLine(0, 0);
            }
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }

        if (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10)
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
    }

    const char* ConsoleWidget::levelToString(ConsoleLogLevel level) const
    {
        switch (level)
        {
            case ConsoleLogLevel::trace:
                return "Trace";
            case ConsoleLogLevel::debug:
                return "Debug";
            case ConsoleLogLevel::info:
                return "Info";
            case ConsoleLogLevel::warn:
                return "Warn";
            case ConsoleLogLevel::error:
                return "Error";
            case ConsoleLogLevel::critical:
                return "Critical";
            default:
                return "?";
        }
    }

    ImVec4 ConsoleWidget::levelToColor(ConsoleLogLevel level) const
    {
        switch (level)
        {
            case ConsoleLogLevel::trace:
                return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
            case ConsoleLogLevel::debug:
                return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
            case ConsoleLogLevel::info:
                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            case ConsoleLogLevel::warn:
                return ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
            case ConsoleLogLevel::error:
                return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            case ConsoleLogLevel::critical:
                return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            default:
                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

} // namespace RealmEngine
