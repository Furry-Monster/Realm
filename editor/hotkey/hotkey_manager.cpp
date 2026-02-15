#include "hotkey_manager.h"

#include "core/log/log_macros.h"

#include <imgui.h>

namespace RealmEngine
{
    bool HotkeyManager::registerHotkey(ImGuiKeyChord chord, Handler handler)
    {
        return registerHotkey(chord, std::move(handler), ImGuiInputFlags_RouteAlways);
    }

    bool HotkeyManager::registerHotkey(ImGuiKeyChord chord, Handler handler, ImGuiInputFlags flags)
    {
        if (!handler)
            return false;

        // Warn on duplicate chord+flags (later entry will shadow the earlier one)
        for (const auto& entry : m_entries)
        {
            if (entry.chord == chord && entry.flags == flags)
            {
                RE_LOG_WARN("Hotkey conflict: chord already registered with same flags");
                break;
            }
        }
        m_entries.push_back({chord, std::move(handler), flags});
        return true;
    }

    void HotkeyManager::process()
    {
        for (const auto& entry : m_entries)
        {
            if (ImGui::Shortcut(entry.chord, entry.flags))
            {
                entry.handler();
                break;
            }
        }
    }

} // namespace RealmEngine
