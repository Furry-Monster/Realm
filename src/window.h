#pragma once

#include <functional>
#include <memory>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace RealmEngine
{
    struct GLFWwindowDeleter
    {
        void operator()(GLFWwindow* window) const
        {
            if (window)
                glfwDestroyWindow(window);
        }
    };

    struct WindowConfig
    {
        int          width        = 1280;
        int          height       = 720;
        std::string  title        = "RealmEngine";
        bool         fullscreen   = false;
        bool         vsync        = true;
        int          msaa_samples = 4;
        GLFWmonitor* monitor      = nullptr; // For fullscreen mode

        WindowConfig& setSize(int w, int h)
        {
            width  = w;
            height = h;
            return *this;
        }
        WindowConfig& setTitle(const std::string& t)
        {
            title = t;
            return *this;
        }
        WindowConfig& setFullscreen(bool f)
        {
            fullscreen = f;
            return *this;
        }
        WindowConfig& setVSync(bool v)
        {
            vsync = v;
            return *this;
        }
        WindowConfig& setMSAA(int samples)
        {
            msaa_samples = samples;
            return *this;
        }
        WindowConfig& setMonitor(GLFWmonitor* m)
        {
            monitor = m;
            return *this;
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

        void initialize();
        void initialize(int width, int height, const std::string& title);
        void initialize(WindowConfig cfg);

        void disposal();

        bool shouldClose() const;
        void pollEvents() const;
        void swapBuffer() const;

        std::string getTitle() const;
        int         getWidth() const;
        int         getHeight() const;
        int         getFramebufferWidth() const;
        int         getFramebufferHeight() const;
        int         getMSAASamples() const;

        bool isHDREnabled() const;
        bool isMSAAEnabled() const;
        bool isVSyncEnabled() const;

        using onResetFunc           = std::function<void()>;
        using onKeyFunc             = std::function<void(int, int, int, int)>;
        using onCharFunc            = std::function<void(unsigned int)>;
        using onCharModsFunc        = std::function<void(int, unsigned int)>;
        using onMouseButtonFunc     = std::function<void(int, int, int)>;
        using onCursorPosFunc       = std::function<void(double, double)>;
        using onCursorEnterFunc     = std::function<void(int)>;
        using onScrollFunc          = std::function<void(double, double)>;
        using onDropFunc            = std::function<void(int, const char**)>;
        using onWindowSizeFunc      = std::function<void(int, int)>;
        using onFramebufferSizeFunc = std::function<void(int, int)>;
        using onWindowCloseFunc     = std::function<void()>;

        void registerOnResetFunc(onResetFunc func) { m_onResetFunc.push_back(func); }
        void registerOnKeyFunc(onKeyFunc func) { m_onKeyFunc.push_back(func); }
        void registerOnCharFunc(onCharFunc func) { m_onCharFunc.push_back(func); }
        void registerOnCharModsFunc(onCharModsFunc func) { m_onCharModsFunc.push_back(func); }
        void registerOnMouseButtonFunc(onMouseButtonFunc func) { m_onMouseButtonFunc.push_back(func); }
        void registerOnCursorPosFunc(onCursorPosFunc func) { m_onCursorPosFunc.push_back(func); }
        void registerOnCursorEnterFunc(onCursorEnterFunc func) { m_onCursorEnterFunc.push_back(func); }
        void registerOnScrollFunc(onScrollFunc func) { m_onScrollFunc.push_back(func); }
        void registerOnDropFunc(onDropFunc func) { m_onDropFunc.push_back(func); }
        void registerOnWindowSizeFunc(onWindowSizeFunc func) { m_onWindowSizeFunc.push_back(func); }
        void registerOnFramebufferSizeFunc(onFramebufferSizeFunc func) { m_onFramebufferSizeFunc.push_back(func); }
        void registerOnWindowCloseFunc(onWindowCloseFunc func) { m_onWindowCloseFunc.push_back(func); }

    protected:
        // event handler
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onKey(key, scancode, action, mods);
            }
        }
        static void charCallback(GLFWwindow* window, unsigned int codepoint)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onChar(codepoint);
            }
        }
        static void charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onCharMods(codepoint, mods);
            }
        }
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onMouseButton(button, action, mods);
            }
        }
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onCursorPos(xpos, ypos);
            }
        }
        static void cursorEnterCallback(GLFWwindow* window, int entered)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onCursorEnter(entered);
            }
        }
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onScroll(xoffset, yoffset);
            }
        }
        static void dropCallback(GLFWwindow* window, int count, const char** paths)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onDrop(count, paths);
            }
        }
        static void windowSizeCallback(GLFWwindow* window, int width, int height)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onWindowSize(width, height);
                app->m_width  = width;
                app->m_height = height;
            }
        }
        static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onFramebufferSize(width, height);
                app->m_framebuffer_width  = width;
                app->m_framebuffer_height = height;
            }
        }
        static void windowCloseCallback(GLFWwindow* window)
        {
            Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            if (app)
            {
                app->onWindowClose();
                glfwSetWindowShouldClose(window, true);
            }
        }

        // event exec
        void onReset()
        {
            for (auto& func : m_onResetFunc)
                func();
        }
        void onKey(int key, int scancode, int action, int mods)
        {
            for (auto& func : m_onKeyFunc)
                func(key, scancode, action, mods);
        }
        void onChar(unsigned int codepoint)
        {
            for (auto& func : m_onCharFunc)
                func(codepoint);
        }
        void onCharMods(int codepoint, unsigned int mods)
        {
            for (auto& func : m_onCharModsFunc)
                func(codepoint, mods);
        }
        void onMouseButton(int button, int action, int mods)
        {
            for (auto& func : m_onMouseButtonFunc)
                func(button, action, mods);
        }
        void onCursorPos(double xpos, double ypos)
        {
            for (auto& func : m_onCursorPosFunc)
                func(xpos, ypos);
        }
        void onCursorEnter(int entered)
        {
            for (auto& func : m_onCursorEnterFunc)
                func(entered);
        }
        void onScroll(double xoffset, double yoffset)
        {
            for (auto& func : m_onScrollFunc)
                func(xoffset, yoffset);
        }
        void onDrop(int count, const char** paths)
        {
            for (auto& func : m_onDropFunc)
                func(count, paths);
        }
        void onWindowSize(int width, int height)
        {
            for (auto& func : m_onWindowSizeFunc)
                func(width, height);
        }
        void onFramebufferSize(int width, int height)
        {
            for (auto& func : m_onFramebufferSizeFunc)
                func(width, height);
        }
        void onWindowClose()
        {
            for (auto& func : m_onWindowCloseFunc)
                func();
        }

    private:
        std::unique_ptr<GLFWwindow, GLFWwindowDeleter> m_window;
        std::string                                    m_title {"RealmEngine"};
        int                                            m_width {0};
        int                                            m_height {0};
        int                                            m_framebuffer_width {0};
        int                                            m_framebuffer_height {0};
        int                                            m_msaa_samples {0};
        bool                                           m_vsync {false};

        // events
        std::vector<onResetFunc>           m_onResetFunc;
        std::vector<onKeyFunc>             m_onKeyFunc;
        std::vector<onCharFunc>            m_onCharFunc;
        std::vector<onCharModsFunc>        m_onCharModsFunc;
        std::vector<onMouseButtonFunc>     m_onMouseButtonFunc;
        std::vector<onCursorPosFunc>       m_onCursorPosFunc;
        std::vector<onCursorEnterFunc>     m_onCursorEnterFunc;
        std::vector<onScrollFunc>          m_onScrollFunc;
        std::vector<onDropFunc>            m_onDropFunc;
        std::vector<onWindowSizeFunc>      m_onWindowSizeFunc;
        std::vector<onFramebufferSizeFunc> m_onFramebufferSizeFunc;
        std::vector<onWindowCloseFunc>     m_onWindowCloseFunc;
    };
} // namespace RealmEngine