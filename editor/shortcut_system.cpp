#include "shortcut_system.h"

namespace RealmEngine
{
    void ShortcutSystem::registerShortcut(ImGuiKeyChord chord, Handler handler)
    {
        if (handler)
            m_entries.push_back({chord, std::move(handler)});
    }

    void ShortcutSystem::process()
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
