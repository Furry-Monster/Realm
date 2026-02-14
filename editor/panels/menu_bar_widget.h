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
        std::function<void()>                 on_undo;
        std::function<void()>                 on_redo;
        std::function<void()>                 on_cut;
        std::function<void()>                 on_copy;
        std::function<void()>                 on_paste;
        std::function<void()>                 on_delete;
        std::function<void()>                 on_duplicate;
        std::function<void()>                 on_project_settings;
        std::function<void()>                 on_preferences;
        std::function<bool()>                 can_undo;
        std::function<bool()>                 can_redo;
        std::function<bool()>                 can_copy;
        std::function<bool()>                 can_paste;
        std::function<bool()>                 can_delete;
        std::function<bool()>                 can_duplicate;
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
        void renderSettingsMenu() const;

        static const char* panelShortcut(size_t one_based_index);

        MenuBarCallbacks m_callbacks;
    };

} // namespace RealmEngine
