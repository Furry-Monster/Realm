#pragma once

#include <functional>
#include <vector>

#include "widget.h"

namespace RealmEngine
{
    class Widget;

    struct MenuBarCallbacks
    {
        std::function<void()>                 on_new_scene;
        std::function<void()>                 on_open_scene;
        std::function<void()>                 on_reload_scene;
        std::function<void()>                 on_save_scene;
        std::function<void()>                 on_save_scene_as;
        std::function<void()>                 on_exit;
        std::function<std::vector<Widget*>()> get_view_panels;
    };

    class MenuBarWidget : public Widget
    {
    public:
        explicit MenuBarWidget(MenuBarCallbacks callbacks);
        ~MenuBarWidget() override = default;

        MenuBarWidget(const MenuBarWidget&)            = delete;
        MenuBarWidget& operator=(const MenuBarWidget&) = delete;
        MenuBarWidget(MenuBarWidget&&)                 = delete;
        MenuBarWidget& operator=(MenuBarWidget&&)      = delete;

        void render() override;

    private:
        void renderFileMenu() const;
        void renderEditMenu() const;
        void renderViewMenu() const;

        static const char* panelShortcut(size_t one_based_index);

        MenuBarCallbacks m_callbacks;
    };

} // namespace RealmEngine
