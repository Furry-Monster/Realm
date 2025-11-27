#include "editor/widgets/menu_bar_widget.h"

#include <imgui.h>

namespace RealmEngine
{
    MenuBarWidget::MenuBarWidget() : Widget("MenuBar") {}

    void MenuBarWidget::render()
    {
        if (ImGui::BeginMainMenuBar())
        {
            renderFileMenu();
            renderEditMenu();
            renderViewMenu();

            ImGui::EndMainMenuBar();
        }
    }

    void MenuBarWidget::renderFileMenu()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                // TODO: 创建新场景
            }

            if (ImGui::MenuItem("Open Scene..."))
            {
                // TODO: 打开场景文件
            }

            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                // TODO: 保存场景
            }

            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
            {
                // TODO: 另存为场景
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                // TODO: 退出编辑器
            }

            ImGui::EndMenu();
        }
    }

    void MenuBarWidget::renderEditMenu()
    {
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false))
            {
                // TODO: 撤销
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
            {
                // TODO: 重做
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Cut", "Ctrl+X", false, false))
            {
                // TODO: 剪切
            }

            if (ImGui::MenuItem("Copy", "Ctrl+C", false, false))
            {
                // TODO: 复制
            }

            if (ImGui::MenuItem("Paste", "Ctrl+V", false, false))
            {
                // TODO: 粘贴
            }

            ImGui::EndMenu();
        }
    }

    void MenuBarWidget::renderViewMenu()
    {
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Scene Hierarchy"))
            {
                // TODO: 显示/隐藏场景层次面板
            }

            if (ImGui::MenuItem("Properties"))
            {
                // TODO: 显示/隐藏属性面板
            }

            if (ImGui::MenuItem("Viewport"))
            {
                // TODO: 显示/隐藏视口面板
            }

            ImGui::EndMenu();
        }
    }

} // namespace RealmEngine
