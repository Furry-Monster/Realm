#include "window.h"
#include "utils.h"

namespace RealmEngine
{

    void Window::initialize() { initialize(WindowConfig {}); }

    void Window::initialize(const WindowConfig cfg)
    {
        m_width        = cfg.width;
        m_height       = cfg.height;
        m_title        = cfg.title;
        m_msaa_samples = cfg.msaa_samples;
        m_vsync        = cfg.vsync;

        if (!glfwInit())
        {
            fatal("Failed to initialize glfw");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if (cfg.msaa_samples > 0)
            glfwWindowHint(GLFW_SAMPLES, m_msaa_samples);

        GLFWmonitor* monitor = cfg.fullscreen ? (cfg.monitor ? cfg.monitor : glfwGetPrimaryMonitor()) : nullptr;

        GLFWwindow* raw_window_ptr = glfwCreateWindow(m_width, m_height, m_title.c_str(), monitor, nullptr);
        if (!raw_window_ptr)
        {
            glfwTerminate();
            fatal("Failed to create window in glfw");
            return;
        }
        m_window.reset(raw_window_ptr);
        glfwMakeContextCurrent(m_window.get());

        if (!gladLoadGL(glfwGetProcAddress))
        {
            m_window.reset();
            glfwTerminate();
            fatal("Failed to initalize GLAD");
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

        glfwGetFramebufferSize(m_window.get(), &m_framebuffer_width, &m_framebuffer_height);
        glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
        glfwSwapInterval(m_vsync ? 1 : 0);
        if (m_msaa_samples > 0)
            glEnable(GL_MULTISAMPLE);

        info("GLFW window initialized.");
    }

    void Window::initialize(int width, int height, const std::string& title)
    {
        WindowConfig cfg;
        cfg.width  = width;
        cfg.height = height;
        cfg.title  = title;
        initialize(cfg);
    }

    void Window::initialize(int width, int height, const std::string& title, bool fullscreen, bool vsync)
    {
        WindowConfig cfg;
        cfg.width      = width;
        cfg.height     = height;
        cfg.title      = title;
        cfg.fullscreen = fullscreen;
        cfg.vsync      = vsync;
        initialize(cfg);
    }

    void Window::destroy()
    {
        info("GLFW window destoyed.");
        m_window.reset();
        glfwTerminate();
    }

    bool Window::shouldClose() const { return glfwWindowShouldClose(m_window.get()); }
    void Window::pollEvents() const { glfwPollEvents(); }
    void Window::swapBuffer() const { glfwSwapBuffers(m_window.get()); }

    int Window::getWidth() const { return m_width; }
    int Window::getHeight() const { return m_height; }
    int Window::getFramebufferWidth() const { return m_framebuffer_width; }
    int Window::getFramebufferHeight() const { return m_framebuffer_height; }

} // namespace RealmEngine