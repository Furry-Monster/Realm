#include "platform/input/input.h"

#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "platform/window/window.h"

namespace RealmEngine
{
    void Input::onKey(int key, int /*scancode*/, int action, int /*mods*/)
    {
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
                case GLFW_KEY_W:
                    m_curr_command |= static_cast<Command>(BindableCommand::FORWARD);
                    break;
                case GLFW_KEY_S:
                    m_curr_command |= static_cast<Command>(BindableCommand::BACKWARD);
                    break;
                case GLFW_KEY_A:
                    m_curr_command |= static_cast<Command>(BindableCommand::LEFT);
                    break;
                case GLFW_KEY_D:
                    m_curr_command |= static_cast<Command>(BindableCommand::RIGHT);
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    m_curr_command |= static_cast<Command>(BindableCommand::SPRINT);
                    break;
                case GLFW_KEY_LEFT_ALT:
                    m_curr_command |= static_cast<Command>(BindableCommand::FOCUS);
                    m_focus = true;
                    setCursorHidden(true);
                    break;
                default:
                    break;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            switch (key)
            {
                case GLFW_KEY_W:
                    m_curr_command &= (COMMAND_COMPLETE_MASK ^ static_cast<Command>(BindableCommand::FORWARD));
                    break;
                case GLFW_KEY_S:
                    m_curr_command &= (COMMAND_COMPLETE_MASK ^ static_cast<Command>(BindableCommand::BACKWARD));
                    break;
                case GLFW_KEY_A:
                    m_curr_command &= (COMMAND_COMPLETE_MASK ^ static_cast<Command>(BindableCommand::LEFT));
                    break;
                case GLFW_KEY_D:
                    m_curr_command &= (COMMAND_COMPLETE_MASK ^ static_cast<Command>(BindableCommand::RIGHT));
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    m_curr_command &= (COMMAND_COMPLETE_MASK ^ static_cast<Command>(BindableCommand::SPRINT));
                    break;
                case GLFW_KEY_LEFT_ALT:
                    m_curr_command &= (COMMAND_COMPLETE_MASK ^ static_cast<Command>(BindableCommand::FOCUS));
                    m_focus = false;
                    setCursorHidden(false);
                    break;
                default:
                    break;
            }
        }
    }

    void Input::onCursorPos(double x, double y)
    {
        if (m_focus)
        {
            m_cursor_delta_x = x - m_last_cursor_x;
            m_cursor_delta_y = y - m_last_cursor_y;
        }

        m_last_cursor_x = x;
        m_last_cursor_y = y;
    }

    void Input::onMouseButton(int button, int /*action*/, int /*mods*/)
    {
        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                break;
            default:
                break;
        }
    }

    void Input::initialize(EventBus& event_bus, Window& window)
    {
        m_window = &window;

        m_subscriptions.push_back(
            event_bus.subscribe<KeyEvent>([this](const KeyEvent& e) { onKey(e.key, e.scancode, e.action, e.mods); }));

        m_subscriptions.push_back(
            event_bus.subscribe<CursorPosEvent>([this](const CursorPosEvent& e) { onCursorPos(e.x, e.y); }));

        m_subscriptions.push_back(event_bus.subscribe<MouseButtonEvent>(
            [this](const MouseButtonEvent& e) { onMouseButton(e.button, e.action, e.mods); }));
    }

    void Input::tick()
    {
        m_cursor_delta_x = 0.0;
        m_cursor_delta_y = 0.0;

        if (!m_focus)
            resetCommand();
    }

    void Input::disposal(EventBus& event_bus)
    {
        for (auto id : m_subscriptions)
            event_bus.unsubscribe(id);
        m_subscriptions.clear();

        m_window         = nullptr;
        m_cursor_delta_x = 0.0;
        m_cursor_delta_y = 0.0;
    }

    void Input::resetCommand() { m_curr_command = 0; }

    Command Input::getCurrentCommand() const { return m_curr_command; }

    void Input::setCursorHidden(bool hidden)
    {
        if (m_window)
            m_window->setCursorMode(hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
} // namespace RealmEngine
