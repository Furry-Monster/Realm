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
        FORWARD  = 1u << 0,  // default: W
        BACKWARD = 1u << 1,  // default: S
        LEFT     = 1u << 2,  // default: A
        RIGHT    = 1u << 3,  // default: D
        SPRINT   = 1u << 4,  // default: left shift
        FOCUS    = 1u << 5,  // default: left alt
        INVALID  = 1u << 31, // lost focus
    };

    static constexpr Command COMMAND_COMPLETE_MASK = 0xFFFFFFFF;

    class Input
    {
    public:
        void initialize(EventBus& event_bus, Window& window);
        void tick();
        void disposal(EventBus& event_bus);

        // --- Command system ---

        void    resetCommand();
        Command getCurrentCommand() const;

        /// Check whether a specific command is currently active.
        bool isCommandActive(BindableCommand command) const;

        // --- Cursor delta ---

        double getCursorDeltaX() const;
        double getCursorDeltaY() const;

        void setCursorHidden(bool hidden);

        // --- Key binding management ---

        /// Bind a GLFW keyboard key to a command.
        void bindKey(int glfw_key, BindableCommand command);

        /// Bind a GLFW mouse button to a command.
        void bindMouseButton(int glfw_button, BindableCommand command);

        /// Remove the binding for a keyboard key.
        void unbindKey(int glfw_key);

        /// Remove the binding for a mouse button.
        void unbindMouseButton(int glfw_button);

        /// Restore default key bindings (WASD + LShift + LAlt).
        void resetToDefaultBindings();

        /// Get the GLFW key code currently bound to a command. Returns -1 if not found.
        int getKeyForCommand(BindableCommand command) const;

        /// Get all current keyboard bindings.
        const std::unordered_map<int, BindableCommand>& getKeyBindings() const;

        /// Get all current mouse button bindings.
        const std::unordered_map<int, BindableCommand>& getMouseBindings() const;

        // --- Raw input state queries ---

        /// Check if a specific keyboard key is currently pressed.
        bool isKeyPressed(int glfw_key) const;

        /// Check if a specific mouse button is currently pressed.
        bool isMouseButtonPressed(int glfw_button) const;

        /// Get the current cursor position.
        void getCursorPosition(double& x, double& y) const;

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

        Window* m_window {nullptr};

        // Data-driven key bindings: GLFW key/button code -> command
        std::unordered_map<int, BindableCommand> m_key_bindings;
        std::unordered_map<int, BindableCommand> m_mouse_bindings;

        // Raw key/button state tracking
        std::unordered_map<int, bool> m_key_states;
        std::unordered_map<int, bool> m_mouse_button_states;

        // EventBus subscription handles
        std::vector<EventBus::HandlerId> m_subscriptions;
    };
} // namespace RealmEngine
