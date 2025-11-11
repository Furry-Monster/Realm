#pragma once

#include <cstdint>

namespace RealmEngine
{
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

    static Command COMMAND_COMPLETE_MASK = 0xFFFFFFFF;

    class Input
    {
    public:
        void onKey(int key, int scancode, int action, int mods);
        void onCursorPos(double current_cursor_x, double current_cursor_y);
        void onMouseButton(int button, int action, int mods);

        void initialize();
        void tick();
        void disposal();

        void    resetCommand();
        Command getCurrentCommand() const;

        void setCursorHidden(bool hidden);

        int m_cursor_delta_x {0};
        int m_cursor_delta_y {0};

    private:
        bool    m_focus {false};
        Command m_curr_command {0};

        int m_last_cursor_x {0};
        int m_last_cursor_y {0};
    };
} // namespace RealmEngine
