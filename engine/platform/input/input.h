#pragma once

#include <cstdint>
#include <vector>

#include "core/event/event_bus.h"

namespace RealmEngine
{
    class Window;

    using Command = unsigned int;

    enum class BindableCommand : uint32_t
    {
        FORWARD  = 1u << 0,  // w
        BACKWARD = 1u << 1,  // s
        LEFT     = 1u << 2,  // a
        RIGHT    = 1u << 3,  // d
        SPRINT   = 1u << 4,  // left shift
        FOCUS    = 1u << 5,  // left alt
        INVALID  = 1u << 31, // lost focus
    };

    static constexpr Command COMMAND_COMPLETE_MASK = 0xFFFFFFFF;

    class Input
    {
    public:
        void initialize(EventBus& event_bus, Window& window);
        void tick();
        void disposal(EventBus& event_bus);

        void    resetCommand();
        Command getCurrentCommand() const;

        void setCursorHidden(bool hidden);

        double m_cursor_delta_x {0.0};
        double m_cursor_delta_y {0.0};

    private:
        void onKey(int key, int scancode, int action, int mods);
        void onCursorPos(double x, double y);
        void onMouseButton(int button, int action, int mods);

        bool    m_focus {false};
        Command m_curr_command {0};

        double m_last_cursor_x {0.0};
        double m_last_cursor_y {0.0};

        Window* m_window {nullptr};

        // EventBus subscription handles
        std::vector<EventBus::HandlerId> m_subscriptions;
    };
} // namespace RealmEngine
