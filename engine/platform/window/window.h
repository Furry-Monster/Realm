#pragma once

#include <memory>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace RealmEngine
{
    class EventBus;
    struct WindowConfig;

    struct GLFWwindowDeleter
    {
        void operator()(GLFWwindow* window) const
        {
            if (window)
                glfwDestroyWindow(window);
        }
    };

    class Window
    {
    public:
        Window()           = default;
        ~Window() noexcept = default;

        Window(const Window& that)            = delete;
        Window(Window&& that)                 = delete;
        Window& operator=(const Window& that) = delete;
        Window& operator=(Window&& that)      = delete;

        void initialize(EventBus& event_bus, const WindowConfig& config);
        void disposal();

        bool shouldClose() const;
        void pollEvents() const;
        void swapBuffer() const;

        std::string getTitle() const;
        int         getWidth() const;
        int         getHeight() const;
        int         getMSAASamples() const;
        GLFWwindow* getGLFWWindow() const;

        bool isMSAAEnabled() const;
        bool isVSyncEnabled() const;

        void setCursorMode(int mode) const;

    private:
        // GLFW static callbacks that publish events through EventBus
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void charCallback(GLFWwindow* window, unsigned int codepoint);
        static void charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void cursorEnterCallback(GLFWwindow* window, int entered);
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        static void dropCallback(GLFWwindow* window, int count, const char** paths);
        static void windowSizeCallback(GLFWwindow* window, int width, int height);
        static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
        static void windowCloseCallback(GLFWwindow* window);

        std::unique_ptr<GLFWwindow, GLFWwindowDeleter> m_window;
        EventBus*                                      m_event_bus {nullptr};
        std::string                                    m_title {"RealmEngine"};
        int                                            m_width {0};
        int                                            m_height {0};
        int                                            m_msaa_samples {0};
        bool                                           m_vsync {false};
    };
} // namespace RealmEngine
