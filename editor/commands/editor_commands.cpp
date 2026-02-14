#include "editor_commands.h"

#include "bridge/editor_engine_bridge.h"
#include "core/log/log_macros.h"
#include "panels/file_dialog_widget.h"
#include "widget.h"

#include <filesystem>

namespace RealmEngine
{
    NewSceneCommand::NewSceneCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void NewSceneCommand::execute()
    {
        auto new_scene = m_bridge->createDefaultScene();
        if (new_scene)
        {
            m_bridge->setCurrentScene(new_scene);
            RE_LOG_INFO("New scene created");
        }
    }

    OpenSceneCommand::OpenSceneCommand(EditorEngineBridge& bridge, FileDialogWidget* file_dialog) :
        m_bridge(&bridge), m_file_dialog(file_dialog)
    {}

    void OpenSceneCommand::execute()
    {
        if (m_file_dialog)
        {
            std::filesystem::path initial_path = m_bridge->getConfigRootFolder();
            m_file_dialog->open(FileDialogWidget::Mode::Open, "Open Scene", ".json", initial_path);
        }
    }

    SaveSceneCommand::SaveSceneCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void SaveSceneCommand::execute()
    {
        if (!m_bridge->getCurrentScene())
        {
            RE_LOG_INFO("No scene to save");
            return;
        }
        std::filesystem::path scene_file = m_bridge->getSceneFileFromConfig();
        if (m_bridge->saveCurrentScene(scene_file.string()))
            RE_LOG_INFO("Scene saved to: " + scene_file.string());
        else
            RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
    }

    SaveSceneAsCommand::SaveSceneAsCommand(EditorEngineBridge& bridge, FileDialogWidget* file_dialog) :
        m_bridge(&bridge), m_file_dialog(file_dialog)
    {}

    void SaveSceneAsCommand::execute()
    {
        if (m_file_dialog && m_bridge->getCurrentScene())
        {
            std::filesystem::path initial_path = m_bridge->getConfigRootFolder();
            m_file_dialog->open(FileDialogWidget::Mode::Save, "Save Scene As", ".json", initial_path);
        }
    }

    ReloadSceneCommand::ReloadSceneCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void ReloadSceneCommand::execute()
    {
        if (!m_bridge->getCurrentScene())
            return;

        std::filesystem::path scene_file = m_bridge->getSceneFileFromConfig();
        if (!std::filesystem::exists(scene_file))
        {
            RE_LOG_WARN("Scene file not found: " + scene_file.string());
            return;
        }

        auto loaded = m_bridge->loadScene(scene_file.string());
        if (loaded)
        {
            m_bridge->setCurrentScene(loaded);
            m_bridge->initializeCameraForScene(loaded);
            RE_LOG_INFO("Scene reloaded from: " + scene_file.string());
        }
        else
        {
            RE_LOG_ERROR("Failed to reload scene from: " + scene_file.string());
        }
    }

    ExitCommand::ExitCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void ExitCommand::execute() { m_bridge->requestWindowClose(); }

    TogglePanelCommand::TogglePanelCommand(std::vector<std::shared_ptr<Widget>>* widgets, size_t index) :
        m_widgets(widgets), m_index(index)
    {}

    void TogglePanelCommand::execute()
    {
        if (m_widgets && m_index < m_widgets->size())
        {
            auto& w = (*m_widgets)[m_index];
            if (w)
                w->setOpen(!w->isOpen());
        }
    }

} // namespace RealmEngine
