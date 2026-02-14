#include "panels/profiler_widget.h"

#include "core/debug/debug_console.h"

#include <imgui.h>

namespace RealmEngine
{
    ProfilerWidget::ProfilerWidget() : Widget("Profiler")
    {
        for (int i = 0; i < HISTORY_SIZE; ++i) // NOLINT(modernize-loop-convert)
            m_frame_time_history[i] = 0.0f;
    }

    void ProfilerWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_name.c_str(), &m_open))
        {
            ImGui::End();
            return;
        }

        const FrameStats& stats = EditorConsole::instance().getFrameStats();

        m_frame_time_history[m_history_index] = static_cast<float>(stats.frame_time_ms);
        m_history_index                       = (m_history_index + 1) % HISTORY_SIZE;

        ImGui::Text("Frame Time: %.2f ms", stats.frame_time_ms);
        ImGui::Text("FPS: %.1f", stats.fps);
        ImGui::Text("Draw Calls: %d", stats.draw_calls);
        ImGui::Text("Triangles: %d", stats.triangle_count);
        ImGui::Text("Memory (RSS): %.1f MB", static_cast<float>(stats.memory_rss_kb) / 1024.0f);

        ImGui::Separator();
        ImGui::PlotLines("Frame Time (ms)",
                         m_frame_time_history,
                         HISTORY_SIZE,
                         m_history_index,
                         nullptr,
                         0.0f,
                         50.0f,
                         ImVec2(0, 80));

        ImGui::End();
    }

} // namespace RealmEngine
