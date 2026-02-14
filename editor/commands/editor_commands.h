#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "command.h"

namespace RealmEngine
{
    class EditorEngineBridge;
    class FileDialogWidget;
    class Widget;

    class NewSceneCommand : public ICommand
    {
    public:
        explicit NewSceneCommand(EditorEngineBridge& bridge);
        void execute() override;

    private:
        EditorEngineBridge* m_bridge;
    };

    class OpenSceneCommand : public ICommand
    {
    public:
        OpenSceneCommand(EditorEngineBridge& bridge, FileDialogWidget* file_dialog);
        void execute() override;

    private:
        EditorEngineBridge* m_bridge;
        FileDialogWidget*   m_file_dialog;
    };

    class SaveSceneCommand : public ICommand
    {
    public:
        explicit SaveSceneCommand(EditorEngineBridge& bridge);
        void execute() override;

    private:
        EditorEngineBridge* m_bridge;
    };

    class SaveSceneAsCommand : public ICommand
    {
    public:
        SaveSceneAsCommand(EditorEngineBridge& bridge, FileDialogWidget* file_dialog);
        void execute() override;

    private:
        EditorEngineBridge* m_bridge;
        FileDialogWidget*   m_file_dialog;
    };

    class ReloadSceneCommand : public ICommand
    {
    public:
        explicit ReloadSceneCommand(EditorEngineBridge& bridge);
        void execute() override;

    private:
        EditorEngineBridge* m_bridge;
    };

    class ExitCommand : public ICommand
    {
    public:
        explicit ExitCommand(EditorEngineBridge& bridge);
        void execute() override;

    private:
        EditorEngineBridge* m_bridge;
    };

    class TogglePanelCommand : public ICommand
    {
    public:
        TogglePanelCommand(std::vector<std::shared_ptr<Widget>>* widgets, size_t index);
        void execute() override;

    private:
        std::vector<std::shared_ptr<Widget>>* m_widgets;
        size_t                                m_index;
    };

} // namespace RealmEngine
