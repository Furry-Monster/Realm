#include "input.h"

#include <memory>
#include <utility>

#include "global_context.h"
#include "window.h"

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

    void Input::onCursorPos(double current_cursor_x, double current_cursor_y)
    {
        if (m_focus)
        {
            m_cursor_delta_x = current_cursor_x - m_last_cursor_x;
            m_cursor_delta_y = current_cursor_y - m_last_cursor_y;
        }

        m_last_cursor_x = current_cursor_x;
        m_last_cursor_y = current_cursor_y;
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

    void Input::initialize()
    {
        std::shared_ptr<Window> window = g_context.m_window;

        window->registerOnKeyFunc([this](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) {
            onKey(std::forward<decltype(PH1)>(PH1),
                  std::forward<decltype(PH2)>(PH2),
                  std::forward<decltype(PH3)>(PH3),
                  std::forward<decltype(PH4)>(PH4));
        });

        window->registerOnCursorPosFunc([this](auto&& PH1, auto&& PH2) {
            onCursorPos(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
        });

        window->registerOnMouseButtonFunc([this](auto&& PH1, auto&& PH2, auto&& PH3) {
            onMouseButton(
                std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3));
        });
    }

    void Input::tick()
    {
        m_cursor_delta_x = 0;
        m_cursor_delta_y = 0;

        if (!m_focus)
            resetCommand();
    }

    void Input::disposal()
    {
        m_cursor_delta_x = 0;
        m_cursor_delta_y = 0;
    }

    void Input::resetCommand() { m_curr_command = 0; }

    Command Input::getCurrentCommand() const { return m_curr_command; }

    void Input::setCursorHidden(bool hidden)
    {
        if (auto window = g_context.m_window)
        {
            window->setCursorMode(hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
    }
} // namespace RealmEngine
