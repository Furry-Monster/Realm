#pragma once

#include <memory>
#include <string>

namespace RealmEngine
{
    class EventBus;
    struct WindowConfig;
    struct WindowImpl;

    enum class CursorMode
    {
        Normal,
        Hidden,
        Disabled
    };

    class Window
    {
    public:
        Window();

        Window(const Window&)            = delete;
        Window(Window&&)                 = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&)      = delete;

        ~Window() noexcept;

        void initialize(EventBus& event_bus, const WindowConfig& config);
        void disposal();

        bool shouldClose() const;
        void pollEvents() const;
        void swapBuffer() const;
        void requestClose();

        std::string getTitle() const;
        int         getWidth() const;
        int         getHeight() const;
        int         getMSAASamples() const;
        double      getTime() const;

        bool isMSAAEnabled() const;
        bool isVSyncEnabled() const;

        void setCursorMode(CursorMode mode) const;

        void* getNativeHandle() const;

        static void* getCurrentContext();
        static void  setCurrentContext(void* ctx);

    private:
        std::unique_ptr<WindowImpl> m_impl;
    };

} // namespace RealmEngine
