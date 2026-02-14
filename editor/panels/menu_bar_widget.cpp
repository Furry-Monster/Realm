#include "panels/menu_bar_widget.h"

#include <imgui.h>

namespace RealmEngine
{
    static void menuItem(const char* label, const char* shortcut, const std::function<void()>& action)
    {
        if (ImGui::MenuItem(label, shortcut) && action)
            action();
    }

    MenuBarWidget::MenuBarWidget(MenuBarCallbacks callbacks) : Widget("MenuBar"), m_callbacks(std::move(callbacks)) {}

    const char* MenuBarWidget::panelShortcut(size_t one_based_index)
    {
        static const char* shortcuts[] = {nullptr, "F1", "F2", "F3", "F4", "F5", "F6"};
        return (one_based_index >= 1 && one_based_index <= 6) ? shortcuts[one_based_index] : nullptr;
    }

    void MenuBarWidget::render()
    {
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::BeginMainMenuBar())
        {
            renderFileMenu();
            renderEditMenu();
            renderViewMenu();
            ImGui::EndMainMenuBar();
        }
    }

    void MenuBarWidget::renderFileMenu() const
    {
        if (!ImGui::BeginMenu("File"))
            return;

        menuItem("New Scene", nullptr, m_callbacks.on_new_scene);
        menuItem("Open Scene...", nullptr, m_callbacks.on_open_scene);
        menuItem("Reload Scene", nullptr, m_callbacks.on_reload_scene);
        menuItem("Save Scene", "Ctrl+S", m_callbacks.on_save_scene);
        menuItem("Save Scene As...", "Ctrl+Shift+S", m_callbacks.on_save_scene_as);
        ImGui::Separator();
        menuItem("Exit", nullptr, m_callbacks.on_exit);
        ImGui::EndMenu();
    }

    void MenuBarWidget::renderEditMenu() const
    {
        if (!ImGui::BeginMenu("Edit"))
            return;

        ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
        ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
        ImGui::Separator();
        ImGui::MenuItem("Cut", "Ctrl+X", false, false);
        ImGui::MenuItem("Copy", "Ctrl+C", false, false);
        ImGui::MenuItem("Paste", "Ctrl+V", false, false);
        ImGui::EndMenu();
    }

    void MenuBarWidget::renderViewMenu() const
    {
        if (!ImGui::BeginMenu("View"))
            return;

        if (m_callbacks.get_view_panels)
        {
            std::vector<Widget*> panels = m_callbacks.get_view_panels();
            for (size_t i = 0; i < panels.size(); ++i)
            {
                Widget* panel = panels[i];
                if (!panel)
                    continue;

                bool is_open = panel->isOpen();
                if (ImGui::MenuItem(panel->getName().c_str(), panelShortcut(i + 1), &is_open))
                    panel->setOpen(is_open);
            }
        }
        ImGui::EndMenu();
    }

} // namespace RealmEngine
