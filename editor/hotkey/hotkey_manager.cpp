#include "hotkey_manager.h"

#include "core/base/macros.h"

#include <imgui.h>

namespace RealmEngine
{
    bool HotkeyManager::registerHotkey(const ImGuiKeyChord chord, Handler handler)
    {
        return registerHotkey(chord, std::move(handler), ImGuiInputFlags_RouteAlways);
    }

    bool HotkeyManager::registerHotkey(const ImGuiKeyChord chord, Handler handler, const ImGuiInputFlags flags)
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

    bool HotkeyManager::unregisterHotkey(const ImGuiKeyChord chord, const ImGuiInputFlags flags)
    {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
        {
            if (it->chord == chord && it->flags == flags)
            {
                m_entries.erase(it);
                return true;
            }
        }
        return false;
    }

    void HotkeyManager::process()
    {
        for (const auto& [chord, handler, flags] : m_entries)
        {
            if (ImGui::Shortcut(chord, flags))
            {
                handler();
                break;
            }
        }
    }

} // namespace RealmEngine
