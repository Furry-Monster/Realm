#pragma once

#include <string>
#include <vector>

namespace RealmEngine
{
    // Window events

    struct WindowResizeEvent
    {
        int width;
        int height;
    };

    struct WindowCloseEvent
    {};

    struct FramebufferResizeEvent
    {
        int width;
        int height;
    };

    // Input events

    struct KeyEvent
    {
        int key;
        int scancode;
        int action;
        int mods;
    };

    struct CharEvent
    {
        unsigned int codepoint;
    };

    struct CharModsEvent
    {
        unsigned int codepoint;
        int          mods;
    };

    struct MouseButtonEvent
    {
        int button;
        int action;
        int mods;
    };

    struct CursorPosEvent
    {
        double x;
        double y;
    };

    struct CursorEnterEvent
    {
        bool entered;
    };

    struct ScrollEvent
    {
        double x_offset;
        double y_offset;
    };

    struct DropEvent
    {
        std::vector<std::string> paths;
    };

} // namespace RealmEngine
