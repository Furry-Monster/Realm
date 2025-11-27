#pragma once

#include "editor/widget.h"

namespace RealmEngine
{
    class MenuBarWidget : public Widget
    {
    public:
        MenuBarWidget();
        ~MenuBarWidget() override = default;

        MenuBarWidget(const MenuBarWidget&)            = delete;
        MenuBarWidget& operator=(const MenuBarWidget&) = delete;
        MenuBarWidget(MenuBarWidget&&)                 = default;
        MenuBarWidget& operator=(MenuBarWidget&&)      = default;

        void render() override;

    private:
        void renderFileMenu();
        void renderEditMenu();
        void renderViewMenu();
    };

} // namespace RealmEngine
