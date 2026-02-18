#pragma once

#include <entt/entity/entity.hpp>
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

    // Editor / Scene events (event-driven editor communication)

    class SceneNode;

    struct EntitySelectedEvent
    {
        entt::entity entity {entt::null};
        SceneNode*   selected_node {nullptr};
    };

    struct SceneChangedEvent
    {
        class Scene* old_scene {nullptr};
        class Scene* new_scene {nullptr};
    };

} // namespace RealmEngine
