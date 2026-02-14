#include "scene_commands.h"

#include <filesystem>

#include "core/log/log_macros.h"
#include "engine.h"
#include "panels/file_dialog_widget.h"
#include "platform/window/window.h"
#include "renderer/renderer.h"
#include "resource/config_manager.h"
#include "rhi/rhi_device.h"
#include "scene/components/camera_controller.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"
#include "widget.h"

namespace RealmEngine
{
    namespace
    {
        class NewSceneCommand : public EditorCommand
        {
        public:
            explicit NewSceneCommand(Engine& engine) : m_engine(engine) {}
            void execute() override
            {
                auto new_scene = m_engine.getSceneManager().createDefaultScene(m_engine.getRenderer().getDevice());
                if (new_scene)
                {
                    m_engine.getSceneManager().setCurrentScene(new_scene);
                    RE_LOG_INFO("New scene created");
                }
            }

        private:
            Engine& m_engine;
        };

        class OpenSceneCommand : public EditorCommand
        {
        public:
            OpenSceneCommand(Engine& engine, FileDialogWidget* file_dialog) :
                m_engine(engine), m_file_dialog(file_dialog)
            {}
            void execute() override
            {
                if (m_file_dialog)
                {
                    std::filesystem::path initial_path = m_engine.getConfig().getRootFolder();
                    m_file_dialog->open(FileDialogWidget::Mode::Open, "Open Scene", ".json", initial_path);
                }
            }

        private:
            Engine&           m_engine;
            FileDialogWidget* m_file_dialog;
        };

        class ReloadSceneCommand : public EditorCommand
        {
        public:
            explicit ReloadSceneCommand(Engine& engine) : m_engine(engine) {}
            void execute() override
            {
                SceneManager&  scene_mgr = m_engine.getSceneManager();
                ConfigManager& config    = m_engine.getConfig();
                if (!scene_mgr.getCurrentScene())
                    return;
                std::filesystem::path scene_file = config.getRootFolder() / config.getGamePlayConfig().scene_file;
                if (!std::filesystem::exists(scene_file))
                {
                    RE_LOG_WARN("Scene file not found: " + scene_file.string());
                    return;
                }
                auto loaded = scene_mgr.loadScene(scene_file.string(), m_engine.getRenderer().getDevice());
                if (loaded)
                {
                    scene_mgr.setCurrentScene(loaded);
                    const GamePlayConfig& gp = config.getGamePlayConfig();
                    loaded->getCameraController()->initialize(m_engine.getRenderer().getCamera(),
                                                              m_engine.getInput(),
                                                              gp.camera_mouse_sensitivity,
                                                              gp.camera_move_speed,
                                                              gp.camera_sprint_multiplier);
                    RE_LOG_INFO("Scene reloaded from: " + scene_file.string());
                }
                else
                {
                    RE_LOG_ERROR("Failed to reload scene from: " + scene_file.string());
                }
            }

        private:
            Engine& m_engine;
        };

        class SaveSceneCommand : public EditorCommand
        {
        public:
            explicit SaveSceneCommand(Engine& engine) : m_engine(engine) {}
            void execute() override
            {
                if (!m_engine.getSceneManager().getCurrentScene())
                {
                    RE_LOG_INFO("No scene to save");
                    return;
                }
                ConfigManager&        config     = m_engine.getConfig();
                std::filesystem::path scene_file = config.getRootFolder() / config.getGamePlayConfig().scene_file;
                if (m_engine.getSceneManager().saveCurrentScene(scene_file.string()))
                    RE_LOG_INFO("Scene saved to: " + scene_file.string());
                else
                    RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
            }

        private:
            Engine& m_engine;
        };

        class SaveSceneAsCommand : public EditorCommand
        {
        public:
            SaveSceneAsCommand(Engine& engine, FileDialogWidget* file_dialog) :
                m_engine(engine), m_file_dialog(file_dialog)
            {}
            void execute() override
            {
                if (m_file_dialog && m_engine.getSceneManager().getCurrentScene())
                {
                    std::filesystem::path initial_path = m_engine.getConfig().getRootFolder();
                    m_file_dialog->open(FileDialogWidget::Mode::Save, "Save Scene As", ".json", initial_path);
                }
            }

        private:
            Engine&           m_engine;
            FileDialogWidget* m_file_dialog;
        };

        class ExitCommand : public EditorCommand
        {
        public:
            explicit ExitCommand(Engine& engine) : m_engine(engine) {}
            void execute() override { m_engine.getWindow().requestClose(); }

        private:
            Engine& m_engine;
        };

        class TogglePanelCommand : public EditorCommand
        {
        public:
            TogglePanelCommand(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets, size_t index) :
                m_widgets(std::move(widgets)), m_index(index)
            {}
            void execute() override
            {
                if (m_widgets && m_index < m_widgets->size())
                {
                    auto w = m_widgets->at(m_index);
                    if (w)
                        w->setOpen(!w->isOpen());
                }
            }

        private:
            std::shared_ptr<std::vector<std::shared_ptr<Widget>>> m_widgets;
            size_t                                                m_index;
        };
    } // namespace

    SceneCommands createSceneCommands(Engine& engine, FileDialogWidget* file_dialog)
    {
        SceneCommands cmds;
        cmds.new_scene     = std::make_shared<NewSceneCommand>(engine);
        cmds.open_scene    = std::make_shared<OpenSceneCommand>(engine, file_dialog);
        cmds.reload_scene  = std::make_shared<ReloadSceneCommand>(engine);
        cmds.save_scene    = std::make_shared<SaveSceneCommand>(engine);
        cmds.save_scene_as = std::make_shared<SaveSceneAsCommand>(engine, file_dialog);
        cmds.exit          = std::make_shared<ExitCommand>(engine);
        return cmds;
    }

    std::shared_ptr<EditorCommand>
    createTogglePanelCommand(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets, size_t index)
    {
        return std::make_shared<TogglePanelCommand>(std::move(widgets), index);
    }

} // namespace RealmEngine
