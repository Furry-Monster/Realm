#pragma once

#include <memory>
#include <vector>
#include "editor_command.h"

namespace RealmEngine
{
    class Engine;
    class FileDialogWidget;
    class Widget;

    struct SceneCommands
    {
        std::shared_ptr<EditorCommand> new_scene;
        std::shared_ptr<EditorCommand> open_scene;
        std::shared_ptr<EditorCommand> reload_scene;
        std::shared_ptr<EditorCommand> save_scene;
        std::shared_ptr<EditorCommand> save_scene_as;
        std::shared_ptr<EditorCommand> exit;
    };

    SceneCommands createSceneCommands(Engine& engine, FileDialogWidget* file_dialog);

    std::shared_ptr<EditorCommand>
    createTogglePanelCommand(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets, size_t index);

} // namespace RealmEngine
