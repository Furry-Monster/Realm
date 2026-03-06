#include "platform/window/window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "core/base/macros.h"
#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "functional/resource/config_manager.h"

#include <string>

namespace RealmEngine
{
    struct WindowImpl
    {
        struct Deleter
        {
            void operator()(GLFWwindow* w) const
            {
                if (w)
                    glfwDestroyWindow(w);
            }
        };

        std::unique_ptr<GLFWwindow, Deleter> handle;
        std::string                          title {"RealmEngine"};
        int                                  width {0};
        int                                  height {0};
        int                                  msaa_samples {0};
        bool                                 vsync {false};
    };

    static void keyCallback(GLFWwindow* w, const int key, const int scancode, const int action, const int mods)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(KeyEvent {key, scancode, action, mods});
    }

    static void charCallback(GLFWwindow* w, const unsigned int codepoint)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(CharEvent {codepoint});
    }

    static void charModsCallback(GLFWwindow* w, const unsigned int codepoint, const int mods)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(CharModsEvent {codepoint, mods});
    }

    static void mouseButtonCallback(GLFWwindow* w, const int button, const int action, const int mods)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(MouseButtonEvent {button, action, mods});
    }

    static void cursorPosCallback(GLFWwindow* w, const double xpos, const double ypos)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(CursorPosEvent {xpos, ypos});
    }

    static void cursorEnterCallback(GLFWwindow* w, const int entered)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(CursorEnterEvent {entered != 0});
    }

    static void scrollCallback(GLFWwindow* w, const double xoffset, const double yoffset)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(ScrollEvent {xoffset, yoffset});
    }

    static void dropCallback(GLFWwindow* w, const int count, const char** paths)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
        {
            DropEvent event;
            event.paths.reserve(static_cast<size_t>(count));
            for (int i = 0; i < count; ++i)
                event.paths.emplace_back(paths[i]);
            g_event_bus->publish(event);
        }
    }

    static void windowSizeCallback(GLFWwindow* w, const int width, const int height)
    {
        auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self)
        {
            self->width  = width;
            self->height = height;
            if (g_event_bus)
                g_event_bus->publish(WindowResizeEvent {width, height});
        }
    }

    static void framebufferSizeCallback(GLFWwindow* w, const int width, const int height)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(FramebufferResizeEvent {width, height});
    }

    static void windowCloseCallback(GLFWwindow* w)
    {
        const auto* self = static_cast<WindowImpl*>(glfwGetWindowUserPointer(w));
        if (self && g_event_bus)
            g_event_bus->publish(WindowCloseEvent {});
        glfwSetWindowShouldClose(w, true);
    }

    Window::Window() = default;

    Window::~Window() noexcept
    {
        if (m_impl)
        {
            m_impl.reset();
            glfwTerminate();
        }
    }

    void Window::initialize(const WindowConfig& config)
    {
        m_impl = std::make_unique<WindowImpl>();
        m_impl->width        = config.width;
        m_impl->height       = config.height;
        m_impl->title        = config.title;
        m_impl->msaa_samples = config.msaa_samples;
        m_impl->vsync        = config.vsync;

        if (!glfwInit())
        {
            RE_LOG_FATAL("Failed to initialize glfw");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

        if (config.msaa_samples > 0)
            glfwWindowHint(GLFW_SAMPLES, m_impl->msaa_samples);

        GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
        GLFWwindow*  raw     = glfwCreateWindow(m_impl->width, m_impl->height, m_impl->title.c_str(), monitor, nullptr);
        if (!raw)
        {
            glfwTerminate();
            RE_LOG_FATAL("Failed to create window in glfw");
            return;
        }
        m_impl->handle.reset(raw);
        glfwMakeContextCurrent(m_impl->handle.get());

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            m_impl->handle.reset();
            glfwTerminate();
            RE_LOG_FATAL("Failed to initialize GLAD");
            return;
        }

        glfwSetWindowUserPointer(m_impl->handle.get(), m_impl.get());
        glfwSetKeyCallback(m_impl->handle.get(), keyCallback);
        glfwSetCharCallback(m_impl->handle.get(), charCallback);
        glfwSetCharModsCallback(m_impl->handle.get(), charModsCallback);
        glfwSetMouseButtonCallback(m_impl->handle.get(), mouseButtonCallback);
        glfwSetCursorPosCallback(m_impl->handle.get(), cursorPosCallback);
        glfwSetCursorEnterCallback(m_impl->handle.get(), cursorEnterCallback);
        glfwSetScrollCallback(m_impl->handle.get(), scrollCallback);
        glfwSetDropCallback(m_impl->handle.get(), dropCallback);
        glfwSetWindowSizeCallback(m_impl->handle.get(), windowSizeCallback);
        glfwSetFramebufferSizeCallback(m_impl->handle.get(), framebufferSizeCallback);
        glfwSetWindowCloseCallback(m_impl->handle.get(), windowCloseCallback);

        glfwSwapInterval(m_impl->vsync ? 1 : 0);

        RE_LOG_INFO("GLFW window initialized.");
    }

    void Window::disposal()
    {
        m_impl.reset();
        glfwTerminate();
        RE_LOG_INFO("GLFW window destroyed.");
    }

    bool Window::shouldClose() const { return m_impl && m_impl->handle && glfwWindowShouldClose(m_impl->handle.get()); }

    void Window::pollEvents() const { glfwPollEvents(); }

    void Window::swapBuffer() const
    {
        if (m_impl && m_impl->handle)
            glfwSwapBuffers(m_impl->handle.get());
    }

    void Window::requestClose()
    {
        if (m_impl && m_impl->handle)
            glfwSetWindowShouldClose(m_impl->handle.get(), 1);
    }

    std::string Window::getTitle() const { return m_impl ? m_impl->title : ""; }

    int Window::getWidth() const { return m_impl ? m_impl->width : 0; }

    int Window::getHeight() const { return m_impl ? m_impl->height : 0; }

    int Window::getMSAASamples() const { return m_impl ? m_impl->msaa_samples : 0; }

    double Window::getTime() const { return glfwGetTime(); }

    bool Window::isMSAAEnabled() const { return m_impl && m_impl->msaa_samples > 0; }

    bool Window::isVSyncEnabled() const { return m_impl && m_impl->vsync; }

    void Window::setCursorMode(const CursorMode mode) const
    {
        if (!m_impl || !m_impl->handle)
            return;
        int glfw_mode = GLFW_CURSOR_NORMAL;
        switch (mode)
        {
            case CursorMode::Normal:
                glfw_mode = GLFW_CURSOR_NORMAL;
                break;
            case CursorMode::Hidden:
                glfw_mode = GLFW_CURSOR_HIDDEN;
                break;
            case CursorMode::Disabled:
                glfw_mode = GLFW_CURSOR_DISABLED;
                break;
        }
        glfwSetInputMode(m_impl->handle.get(), GLFW_CURSOR, glfw_mode);
    }

    void* Window::getNativeHandle() const { return m_impl && m_impl->handle ? m_impl->handle.get() : nullptr; }

    void* Window::getCurrentContext() { return glfwGetCurrentContext(); }

    void Window::setCurrentContext(void* ctx) { glfwMakeContextCurrent(static_cast<GLFWwindow*>(ctx)); }

} // namespace RealmEngine
