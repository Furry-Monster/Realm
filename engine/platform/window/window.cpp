#include "platform/window/window.h"

#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "core/log/log_macros.h"
#include "global_context.h"
#include "resource/config_manager.h"

#include <string>
#include <vector>

namespace RealmEngine
{

    void Window::initialize()
    {
        m_event_bus = g_context.m_event_bus;

        const WindowConfig& window_config = g_context.m_config->getWindowConfig();

        m_width        = window_config.width;
        m_height       = window_config.height;
        m_title        = window_config.title;
        m_msaa_samples = window_config.msaa_samples;
        m_vsync        = window_config.vsync;

        if (!glfwInit())
        {
            RE_LOG_FATAL("Failed to initialize glfw");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if (window_config.msaa_samples > 0)
            glfwWindowHint(GLFW_SAMPLES, m_msaa_samples);

        GLFWmonitor* monitor = window_config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        GLFWwindow* raw_window_ptr = glfwCreateWindow(m_width, m_height, m_title.c_str(), monitor, nullptr);
        if (!raw_window_ptr)
        {
            glfwTerminate();
            RE_LOG_FATAL("Failed to create window in glfw");
            return;
        }
        m_window.reset(raw_window_ptr);
        glfwMakeContextCurrent(m_window.get());

        if (!gladLoadGL(glfwGetProcAddress))
        {
            m_window.reset();
            glfwTerminate();
            RE_LOG_FATAL("Failed to initialize GLAD");
            return;
        }

        glfwSetWindowUserPointer(m_window.get(), this);
        glfwSetKeyCallback(m_window.get(), keyCallback);
        glfwSetCharCallback(m_window.get(), charCallback);
        glfwSetCharModsCallback(m_window.get(), charModsCallback);
        glfwSetMouseButtonCallback(m_window.get(), mouseButtonCallback);
        glfwSetCursorPosCallback(m_window.get(), cursorPosCallback);
        glfwSetCursorEnterCallback(m_window.get(), cursorEnterCallback);
        glfwSetScrollCallback(m_window.get(), scrollCallback);
        glfwSetDropCallback(m_window.get(), dropCallback);
        glfwSetWindowSizeCallback(m_window.get(), windowSizeCallback);
        glfwSetFramebufferSizeCallback(m_window.get(), framebufferSizeCallback);
        glfwSetWindowCloseCallback(m_window.get(), windowCloseCallback);

        glViewport(0, 0, m_width, m_height);
        glfwSwapInterval(m_vsync ? 1 : 0);
        if (m_msaa_samples > 0)
            glEnable(GL_MULTISAMPLE);

        RE_LOG_INFO("GLFW window initialized.");
    }

    void Window::disposal()
    {
        m_event_bus.reset();
        m_window.reset();
        glfwTerminate();

        RE_LOG_INFO("GLFW window destroyed.");
    }

    bool Window::shouldClose() const { return glfwWindowShouldClose(m_window.get()); }
    void Window::pollEvents() const { glfwPollEvents(); }
    void Window::swapBuffer() const { glfwSwapBuffers(m_window.get()); }

    std::string Window::getTitle() const { return m_title; }
    int         Window::getWidth() const { return m_width; }
    int         Window::getHeight() const { return m_height; }
    int         Window::getMSAASamples() const { return m_msaa_samples; }
    GLFWwindow* Window::getGLFWWindow() const { return m_window.get(); }

    bool Window::isMSAAEnabled() const { return m_msaa_samples > 0; }
    bool Window::isVSyncEnabled() const { return m_vsync; }

    void Window::setCursorMode(int mode) const
    {
        if (m_window)
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, mode);
    }

    // GLFW callbacks -- publish events through EventBus

    void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(KeyEvent {key, scancode, action, mods});
    }

    void Window::charCallback(GLFWwindow* window, unsigned int codepoint)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(CharEvent {codepoint});
    }

    void Window::charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(CharModsEvent {codepoint, mods});
    }

    void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(MouseButtonEvent {button, action, mods});
    }

    void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(CursorPosEvent {xpos, ypos});
    }

    void Window::cursorEnterCallback(GLFWwindow* window, int entered)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(CursorEnterEvent {entered != 0});
    }

    void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(ScrollEvent {xoffset, yoffset});
    }

    void Window::dropCallback(GLFWwindow* window, int count, const char** paths)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
        {
            DropEvent event;
            event.paths.reserve(count);
            for (int i = 0; i < count; ++i)
                event.paths.emplace_back(paths[i]);
            self->m_event_bus->publish(event);
        }
    }

    void Window::windowSizeCallback(GLFWwindow* window, int width, int height)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self)
        {
            self->m_width  = width;
            self->m_height = height;
            if (self->m_event_bus)
                self->m_event_bus->publish(WindowResizeEvent {width, height});
        }
    }

    void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(FramebufferResizeEvent {width, height});
    }

    void Window::windowCloseCallback(GLFWwindow* window)
    {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_event_bus)
            self->m_event_bus->publish(WindowCloseEvent {});
        glfwSetWindowShouldClose(window, true);
    }

} // namespace RealmEngine
