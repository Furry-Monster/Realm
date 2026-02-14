#pragma once

#include <functional>
#include <vector>

#include <imgui.h>

namespace RealmEngine
{
    class ShortcutSystem
    {
    public:
        using Handler = std::function<void()>;

        ShortcutSystem()  = default;
        ~ShortcutSystem() = default;

        ShortcutSystem(const ShortcutSystem&)            = delete;
        ShortcutSystem& operator=(const ShortcutSystem&) = delete;
        ShortcutSystem(ShortcutSystem&&)                 = delete;
        ShortcutSystem& operator=(ShortcutSystem&&)      = delete;

        void registerShortcut(ImGuiKeyChord chord, Handler handler);
        void process();

    private:
        struct Entry
        {
            ImGuiKeyChord chord;
            Handler       handler;
        };
        std::vector<Entry> m_entries;
    };

} // namespace RealmEngine
