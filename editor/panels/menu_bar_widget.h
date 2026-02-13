#pragma once

#include <memory>
#include <vector>
#include "widget.h"

namespace RealmEngine
{
    class Engine;
    class Widget;
    class FileDialogWidget;

    class MenuBarWidget : public Widget
    {
    public:
        explicit MenuBarWidget(Engine& engine);
        ~MenuBarWidget() override = default;

        MenuBarWidget(const MenuBarWidget&)            = delete;
        MenuBarWidget& operator=(const MenuBarWidget&) = delete;
        MenuBarWidget(MenuBarWidget&&)                = delete;
        MenuBarWidget& operator=(MenuBarWidget&&)      = delete;

        void render() override;

        void setWidgets(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets) { m_widgets = widgets; }
        void setFileDialog(std::shared_ptr<FileDialogWidget> file_dialog) { m_file_dialog = file_dialog; }

    private:
        void renderFileMenu();
        void renderEditMenu();
        void renderViewMenu();

        Engine& m_engine;

        std::shared_ptr<std::vector<std::shared_ptr<Widget>>> m_widgets;
        std::shared_ptr<FileDialogWidget>                     m_file_dialog;
    };

} // namespace RealmEngine
