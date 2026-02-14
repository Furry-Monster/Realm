#include "hotkey_manager.h"

#include <imgui.h>

namespace RealmEngine
{
    void HotkeyManager::registerHotkey(ImGuiKeyChord chord, Handler handler)
    {
        if (handler)
            m_entries.push_back({chord, std::move(handler), ImGuiInputFlags_RouteAlways});
    }

    void HotkeyManager::registerHotkey(ImGuiKeyChord chord, Handler handler, ImGuiInputFlags flags)
    {
        if (handler)
            m_entries.push_back({chord, std::move(handler), flags});
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
