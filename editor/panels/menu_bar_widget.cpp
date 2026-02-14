#include "panels/menu_bar_widget.h"

#include <imgui.h>

namespace RealmEngine
{
    static void menuItem(const char* label, const char* shortcut, const std::function<void()>& action)
    {
        if (ImGui::MenuItem(label, shortcut) && action)
            action();
    }

    static void menuItem(const char* label, const char* shortcut, bool enabled, const std::function<void()>& action)
    {
        if (ImGui::MenuItem(label, shortcut, false, enabled) && action)
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
            renderSettingsMenu();
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

        bool undo_enabled      = m_callbacks.can_undo ? m_callbacks.can_undo() : false;
        bool redo_enabled      = m_callbacks.can_redo ? m_callbacks.can_redo() : false;
        bool copy_enabled      = m_callbacks.can_copy ? m_callbacks.can_copy() : false;
        bool paste_enabled     = m_callbacks.can_paste ? m_callbacks.can_paste() : false;
        bool delete_enabled    = m_callbacks.can_delete ? m_callbacks.can_delete() : false;
        bool duplicate_enabled = m_callbacks.can_duplicate ? m_callbacks.can_duplicate() : false;

        menuItem("Undo", "Ctrl+Z", undo_enabled, m_callbacks.on_undo);
        menuItem("Redo", "Ctrl+Y", redo_enabled, m_callbacks.on_redo);
        ImGui::Separator();
        menuItem("Cut", "Ctrl+X", delete_enabled, m_callbacks.on_cut);
        menuItem("Copy", "Ctrl+C", copy_enabled, m_callbacks.on_copy);
        menuItem("Paste", "Ctrl+V", paste_enabled, m_callbacks.on_paste);
        menuItem("Delete", "Del", delete_enabled, m_callbacks.on_delete);
        menuItem("Duplicate", "Ctrl+D", duplicate_enabled, m_callbacks.on_duplicate);
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

    void MenuBarWidget::renderSettingsMenu() const
    {
        if (!ImGui::BeginMenu("Settings"))
            return;

        menuItem("Project Settings...", nullptr, m_callbacks.on_project_settings);
        menuItem("Preferences...", nullptr, m_callbacks.on_preferences);
        ImGui::EndMenu();
    }

} // namespace RealmEngine
