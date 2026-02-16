#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "core/event/event_bus.h"

namespace RealmEngine
{
    class Window;

    using Command = unsigned int;

    enum class BindableCommand : uint32_t
    {
        FORWARD  = 1u << 0,
        BACKWARD = 1u << 1,
        LEFT     = 1u << 2,
        RIGHT    = 1u << 3,
        SPRINT   = 1u << 4,
        FOCUS    = 1u << 5,
        INVALID  = 1u << 31,
    };

    static constexpr Command COMMAND_COMPLETE_MASK = 0xFFFFFFFF;

    class Input
    {
    public:
        void initialize(EventBus& event_bus, Window& window);
        void tick();
        void disposal(EventBus& event_bus);

        // command state
        void    resetCommand();
        Command getCurrentCommand() const;
        bool    isCommandActive(BindableCommand command) const;

        // cursor
        double getCursorDeltaX() const;
        double getCursorDeltaY() const;
        void   getCursorPosition(double& x, double& y) const;
        void   setCursorHidden(bool hidden);

        // key binding
        void bindKey(int glfw_key, BindableCommand command);
        void bindMouseButton(int glfw_button, BindableCommand command);
        void unbindKey(int glfw_key);
        void unbindMouseButton(int glfw_button);
        void resetToDefaultBindings();

        int getKeyForCommand(BindableCommand command) const; // -1 if unbound
        const std::unordered_map<int, BindableCommand>& getKeyBindings() const;
        const std::unordered_map<int, BindableCommand>& getMouseBindings() const;

        // raw state queries
        bool isKeyPressed(int glfw_key) const;
        bool isMouseButtonPressed(int glfw_button) const;

    private:
        void onKey(int key, int scancode, int action, int mods);
        void onCursorPos(double x, double y);
        void onMouseButton(int button, int action, int mods);

        void applyCommandPress(BindableCommand cmd);
        void applyCommandRelease(BindableCommand cmd);
        void setupDefaultBindings();

        bool    m_focus {false};
        Command m_curr_command {0};

        double m_cursor_delta_x {0.0};
        double m_cursor_delta_y {0.0};
        double m_last_cursor_x {0.0};
        double m_last_cursor_y {0.0};
        bool   m_first_cursor_after_focus {false};

        Window* m_window {nullptr};

        // GLFW key/button -> command mapping
        std::unordered_map<int, BindableCommand> m_key_bindings;
        std::unordered_map<int, BindableCommand> m_mouse_bindings;

        // per-key/button press state
        std::unordered_map<int, bool> m_key_states;
        std::unordered_map<int, bool> m_mouse_button_states;

        std::vector<EventBus::HandlerId> m_subscriptions;
    };
} // namespace RealmEngine
