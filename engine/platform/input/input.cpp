#include "platform/input/input.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "platform/window/window.h"

namespace RealmEngine
{
    void Input::setupDefaultBindings()
    {
        m_key_bindings.clear();
        m_mouse_bindings.clear();

        m_key_bindings[GLFW_KEY_W]          = BindableCommand::FORWARD;
        m_key_bindings[GLFW_KEY_S]          = BindableCommand::BACKWARD;
        m_key_bindings[GLFW_KEY_A]          = BindableCommand::LEFT;
        m_key_bindings[GLFW_KEY_D]          = BindableCommand::RIGHT;
        m_key_bindings[GLFW_KEY_LEFT_SHIFT] = BindableCommand::SPRINT;
        m_key_bindings[GLFW_KEY_LEFT_ALT]   = BindableCommand::FOCUS;
    }

    void Input::applyCommandPress(BindableCommand cmd)
    {
        m_curr_command |= static_cast<Command>(cmd);

        if (cmd == BindableCommand::FOCUS)
        {
            m_focus                    = true;
            m_first_cursor_after_focus = true;
            setCursorHidden(true);
        }
    }

    void Input::applyCommandRelease(BindableCommand cmd)
    {
        m_curr_command &= ~static_cast<Command>(cmd);

        if (cmd == BindableCommand::FOCUS)
        {
            m_focus = false;
            setCursorHidden(false);
        }
    }

    void Input::onKey(const int key, [[maybe_unused]] int scancode, const int action, [[maybe_unused]] int mods)
    {
        if (action == GLFW_PRESS)
            m_key_states[key] = true;
        else if (action == GLFW_RELEASE)
            m_key_states[key] = false;

        const auto it = m_key_bindings.find(key);
        if (it == m_key_bindings.end())
            return;

        if (action == GLFW_PRESS)
            applyCommandPress(it->second);
        else if (action == GLFW_RELEASE)
            applyCommandRelease(it->second);
    }

    void Input::onCursorPos(const double x, const double y)
    {
        if (m_focus)
        {
            if (!m_first_cursor_after_focus)
            {
                m_cursor_delta_x = x - m_last_cursor_x;
                m_cursor_delta_y = y - m_last_cursor_y;
            }
            else
            {
                m_first_cursor_after_focus = false;
            }
        }

        m_last_cursor_x = x;
        m_last_cursor_y = y;
    }

    void Input::onMouseButton(const int button, const int action, [[maybe_unused]] int mods)
    {
        if (action == GLFW_PRESS)
            m_mouse_button_states[button] = true;
        else if (action == GLFW_RELEASE)
            m_mouse_button_states[button] = false;

        const auto it = m_mouse_bindings.find(button);
        if (it == m_mouse_bindings.end())
            return;

        if (action == GLFW_PRESS)
            applyCommandPress(it->second);
        else if (action == GLFW_RELEASE)
            applyCommandRelease(it->second);
    }

    void Input::initialize(Window& window)
    {
        m_window = &window;

        setupDefaultBindings();

        m_subscriptions.push_back(g_event_bus->subscribe<KeyEvent>(
            [this](const KeyEvent& e) { onKey(e.key, e.scancode, e.action, e.mods); }));

        m_subscriptions.push_back(
            g_event_bus->subscribe<CursorPosEvent>([this](const CursorPosEvent& e) { onCursorPos(e.x, e.y); }));

        m_subscriptions.push_back(g_event_bus->subscribe<MouseButtonEvent>(
            [this](const MouseButtonEvent& e) { onMouseButton(e.button, e.action, e.mods); }));
    }

    void Input::tick()
    {
        m_cursor_delta_x = 0.0;
        m_cursor_delta_y = 0.0;

        if (!m_focus)
            resetCommand();
    }

    void Input::disposal()
    {
        for (const auto id : m_subscriptions)
            g_event_bus->unsubscribe(id);
        m_subscriptions.clear();

        m_key_bindings.clear();
        m_mouse_bindings.clear();
        m_key_states.clear();
        m_mouse_button_states.clear();

        m_window         = nullptr;
        m_cursor_delta_x = 0.0;
        m_cursor_delta_y = 0.0;
    }

    void    Input::resetCommand() { m_curr_command = 0; }
    Command Input::getCurrentCommand() const { return m_curr_command; }

    bool Input::isCommandActive(BindableCommand command) const
    {
        return (m_curr_command & static_cast<Command>(command)) != 0;
    }

    double Input::getCursorDeltaX() const { return m_cursor_delta_x; }
    double Input::getCursorDeltaY() const { return m_cursor_delta_y; }

    void Input::getCursorPosition(double& x, double& y) const
    {
        x = m_last_cursor_x;
        y = m_last_cursor_y;
    }

    void Input::setCursorHidden(const bool hidden) const
    {
        if (m_window)
            m_window->setCursorMode(hidden ? CursorMode::Disabled : CursorMode::Normal);
    }

    void Input::bindKey(const int glfw_key, const BindableCommand command) { m_key_bindings[glfw_key] = command; }

    void Input::bindMouseButton(const int glfw_button, const BindableCommand command)
    {
        m_mouse_bindings[glfw_button] = command;
    }

    void Input::unbindKey(const int glfw_key) { m_key_bindings.erase(glfw_key); }

    void Input::unbindMouseButton(const int glfw_button) { m_mouse_bindings.erase(glfw_button); }

    void Input::resetToDefaultBindings() { setupDefaultBindings(); }

    int Input::getKeyForCommand(const BindableCommand command) const
    {
        for (const auto& [key, cmd] : m_key_bindings)
        {
            if (cmd == command)
                return key;
        }
        return -1;
    }

    const std::unordered_map<int, BindableCommand>& Input::getKeyBindings() const { return m_key_bindings; }

    const std::unordered_map<int, BindableCommand>& Input::getMouseBindings() const { return m_mouse_bindings; }

    bool Input::isKeyPressed(const int glfw_key) const
    {
        const auto it = m_key_states.find(glfw_key);
        return it != m_key_states.end() && it->second;
    }

    bool Input::isMouseButtonPressed(const int glfw_button) const
    {
        const auto it = m_mouse_button_states.find(glfw_button);
        return it != m_mouse_button_states.end() && it->second;
    }

} // namespace RealmEngine
