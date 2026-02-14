#include "hotkey_manager.h"

#include <imgui.h>

namespace RealmEngine
{
    void HotkeyManager::registerHotkey(ImGuiKeyChord chord, Handler handler)
    {
        if (handler)
            m_entries.push_back({chord, std::move(handler)});
    }

    void HotkeyManager::process()
    {
        for (const auto& entry : m_entries)
        {
            if (ImGui::Shortcut(entry.chord, ImGuiInputFlags_RouteAlways))
            {
                entry.handler();
                break;
            }
        }
    }

} // namespace RealmEngine
