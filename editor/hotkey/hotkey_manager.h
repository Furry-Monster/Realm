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
        ~HotkeyManager() noexcept = default;

        HotkeyManager(const HotkeyManager&)            = delete;
        HotkeyManager& operator=(const HotkeyManager&) = delete;
        HotkeyManager(HotkeyManager&&)                 = delete;
        HotkeyManager& operator=(HotkeyManager&&)      = delete;

        /// Returns false if the chord+flags combination is already registered (conflict)
        bool registerHotkey(ImGuiKeyChord chord, Handler handler);
        bool registerHotkey(ImGuiKeyChord chord, Handler handler, ImGuiInputFlags flags);
        bool unregisterHotkey(ImGuiKeyChord chord, ImGuiInputFlags flags = ImGuiInputFlags_RouteAlways);
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
