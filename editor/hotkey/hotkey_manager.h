#pragma once

#include <functional>
#include <vector>

#include <imgui.h>

namespace RealmEngine
{
    class HotkeyManager
    {
    public:
        using Handler = std::function<void()>;

        HotkeyManager()  = default;
        ~HotkeyManager() = default;

        HotkeyManager(const HotkeyManager&)            = delete;
        HotkeyManager& operator=(const HotkeyManager&) = delete;
        HotkeyManager(HotkeyManager&&)                 = delete;
        HotkeyManager& operator=(HotkeyManager&&)      = delete;

        void registerHotkey(ImGuiKeyChord chord, Handler handler);
        void registerHotkey(ImGuiKeyChord chord, Handler handler, ImGuiInputFlags flags);
        void process();

    private:
        struct Entry
        {
            ImGuiKeyChord   chord;
            Handler         handler;
            ImGuiInputFlags flags {ImGuiInputFlags_RouteAlways};
        };
        std::vector<Entry> m_entries;
    };

} // namespace RealmEngine
