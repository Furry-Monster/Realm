#include "editor.h"

#include <filesystem>
#include <memory>

#include "bridge/editor_engine_bridge.h"
#include "commands/command_executor.h"
#include "commands/editor_commands.h"
#include "core/log/log_macros.h"
#include "editor_context.h"
#include "engine.h"
#include "panels/asset_browser_widget.h"
#include "panels/console_widget.h"
#include "panels/entity_browser_widget.h"
#include "panels/file_dialog_widget.h"
#include "panels/menu_bar_widget.h"
#include "panels/preferences_widget.h"
#include "panels/profiler_widget.h"
#include "panels/project_settings_widget.h"
#include "panels/properties_widget.h"
#include "panels/scene_hierarchy_widget.h"
#include "platform/window/window.h"
#include "preferences/editor_preferences.h"
#include "scene/scene.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace RealmEngine
{
    Editor::Editor() = default;

    Editor::~Editor() noexcept { shutdown(); }

    void Editor::initialize()
    {
        if (m_initialized)
            return;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        if (!m_engine)
        {
            m_engine = std::make_unique<Engine>();
            m_engine->boot();
        }

        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_engine->getWindow().getNativeHandle()), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        m_context = std::make_shared<EditorContext>();
        m_bridge  = std::make_unique<EditorEngineBridge>(*m_engine);

        std::filesystem::path prefs_path = m_bridge->getConfigRootFolder() / "editor_preferences.json";
        EditorPreferencesManager::load(m_context->getPreferences(), prefs_path);

        switch (m_context->getPreferences().theme)
        {
            case EditorTheme::Dark:
                ImGui::StyleColorsDark();
                break;
            case EditorTheme::Light:
                ImGui::StyleColorsLight();
                break;
            case EditorTheme::Classic:
                ImGui::StyleColorsClassic();
                break;
        }
        io.FontGlobalScale = m_context->getPreferences().font_scale;

        static std::string ini_path_storage;
        ini_path_storage = (m_bridge->getConfigRootFolder() / "imgui.ini").string();
        io.IniFilename   = ini_path_storage.c_str();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        auto file_dialog = std::make_shared<FileDialogWidget>();
        file_dialog->setOnFileSelected([this, file_dialog](const std::filesystem::path& path) {
            if (file_dialog->getMode() == FileDialogWidget::Mode::Open)
            {
                auto loaded = m_bridge->loadScene(path.string());
                if (loaded)
                {
                    m_bridge->setCurrentScene(loaded);
                    m_bridge->initializeCameraForScene(loaded);
                    RE_LOG_INFO("Scene loaded from: " + path.string());
                }
                else
                {
                    RE_LOG_ERROR("Failed to load scene from: " + path.string());
                }
            }
            else
            {
                if (m_bridge->saveCurrentScene(path.string()))
                    RE_LOG_INFO("Scene saved to: " + path.string());
                else
                    RE_LOG_ERROR("Failed to save scene to: " + path.string());
            }
        });

        MenuBarCallbacks menu_callbacks;
        menu_callbacks.on_new_scene  = [this] { m_executor.execute(NewSceneCommand(*m_bridge)); };
        menu_callbacks.on_open_scene = [this, file_dialog] {
            m_executor.execute(OpenSceneCommand(*m_bridge, file_dialog.get()));
        };
        menu_callbacks.on_reload_scene  = [this] { m_executor.execute(ReloadSceneCommand(*m_bridge)); };
        menu_callbacks.on_save_scene    = [this] { m_executor.execute(SaveSceneCommand(*m_bridge)); };
        menu_callbacks.on_save_scene_as = [this, file_dialog] {
            m_executor.execute(SaveSceneAsCommand(*m_bridge, file_dialog.get()));
        };
        menu_callbacks.on_exit      = [this] { m_executor.execute(ExitCommand(*m_bridge)); };
        menu_callbacks.on_undo      = [this] { m_executor.undo(); };
        menu_callbacks.on_redo      = [this] { m_executor.redo(); };
        menu_callbacks.on_cut       = [this] { m_executor.execute(CutEntityCommand(*m_bridge, *m_context)); };
        menu_callbacks.on_copy      = [this] { m_executor.execute(CopyEntityCommand(*m_bridge, *m_context)); };
        menu_callbacks.on_paste     = [this] { m_executor.execute(PasteEntityCommand(*m_bridge, *m_context)); };
        menu_callbacks.on_delete    = [this] { m_executor.execute(DeleteEntityCommand(*m_bridge, *m_context)); };
        menu_callbacks.on_duplicate = [this] { m_executor.execute(DuplicateEntityCommand(*m_bridge, *m_context)); };
        menu_callbacks.can_undo     = [this] { return m_executor.canUndo(); };
        menu_callbacks.can_redo     = [this] { return m_executor.canRedo(); };
        menu_callbacks.can_copy     = [this] { return m_context->hasSelectedNode(); };
        menu_callbacks.can_paste    = [this] { return m_context->hasEntityClipboard(); };
        menu_callbacks.can_delete   = [this] {
            return m_context->hasSelectedNode() && m_bridge->getCurrentScene() &&
                   m_context->getSelectedNode() != m_bridge->getCurrentScene()->getRoot();
        };
        menu_callbacks.can_duplicate = [this] { return m_context->hasSelectedNode(); };

        auto project_settings = std::make_shared<ProjectSettingsWidget>(*m_bridge);
        auto preferences      = std::make_shared<PreferencesWidget>(
            m_context->getPreferences(),
            [this] { ImGui::GetIO().FontGlobalScale = m_context->getPreferences().font_scale; },
            [this] { return m_bridge->getConfigRootFolder() / "editor_preferences.json"; });
        project_settings->setOpen(false);
        preferences->setOpen(false);

        menu_callbacks.on_project_settings = [project_settings] { project_settings->setOpen(true); };
        menu_callbacks.on_preferences      = [preferences] { preferences->setOpen(true); };

        menu_callbacks.get_view_panels = [this] {
            std::vector<Widget*> out;
            for (size_t i = 1; i < m_panels.size(); ++i)
            {
                if (m_panels[i])
                    out.push_back(m_panels[i].get());
            }
            return out;
        };

        m_panels.push_back(std::make_shared<MenuBarWidget>(std::move(menu_callbacks)));
        m_panels.push_back(std::make_shared<SceneHierarchyWidget>(m_context, *m_bridge));
        m_panels.push_back(std::make_shared<PropertiesWidget>(m_context, *m_bridge));
        m_panels.push_back(std::make_shared<EntityBrowserWidget>(m_context, *m_bridge));
        m_panels.push_back(std::make_shared<ConsoleWidget>());
        m_panels.push_back(std::make_shared<ProfilerWidget>());
        m_panels.push_back(std::make_shared<AssetBrowserWidget>(*m_bridge));
        m_panels.push_back(project_settings);
        m_panels.push_back(preferences);
        m_panels.push_back(file_dialog);

        auto& hotkeys = m_context->getHotkeyManager();
        hotkeys.registerHotkey(ImGuiMod_Ctrl | ImGuiKey_N, [this] { m_executor.execute(NewSceneCommand(*m_bridge)); });
        hotkeys.registerHotkey(ImGuiMod_Ctrl | ImGuiKey_O, [this, file_dialog] {
            m_executor.execute(OpenSceneCommand(*m_bridge, file_dialog.get()));
        });
        hotkeys.registerHotkey(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, [this, file_dialog] {
            m_executor.execute(SaveSceneAsCommand(*m_bridge, file_dialog.get()));
        });
        hotkeys.registerHotkey(ImGuiMod_Ctrl | ImGuiKey_S, [this] { m_executor.execute(SaveSceneCommand(*m_bridge)); });
        hotkeys.registerHotkey(ImGuiMod_Alt | ImGuiKey_F4, [this] { m_executor.execute(ExitCommand(*m_bridge)); });

        constexpr auto edit_flags = ImGuiInputFlags_RouteGlobal;
        hotkeys.registerHotkey(ImGuiMod_Ctrl | ImGuiKey_Z, [this] { m_executor.undo(); }, edit_flags);
        hotkeys.registerHotkey(ImGuiMod_Ctrl | ImGuiKey_Y, [this] { m_executor.redo(); }, edit_flags);
        hotkeys.registerHotkey(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, [this] { m_executor.redo(); }, edit_flags);
        hotkeys.registerHotkey(
            ImGuiMod_Ctrl | ImGuiKey_X,
            [this] { m_executor.execute(CutEntityCommand(*m_bridge, *m_context)); },
            edit_flags);
        hotkeys.registerHotkey(
            ImGuiMod_Ctrl | ImGuiKey_C,
            [this] { m_executor.execute(CopyEntityCommand(*m_bridge, *m_context)); },
            edit_flags);
        hotkeys.registerHotkey(
            ImGuiMod_Ctrl | ImGuiKey_V,
            [this] { m_executor.execute(PasteEntityCommand(*m_bridge, *m_context)); },
            edit_flags);
        hotkeys.registerHotkey(
            ImGuiMod_Ctrl | ImGuiKey_D,
            [this] { m_executor.execute(DuplicateEntityCommand(*m_bridge, *m_context)); },
            edit_flags);
        hotkeys.registerHotkey(
            ImGuiKey_Delete, [this] { m_executor.execute(DeleteEntityCommand(*m_bridge, *m_context)); }, edit_flags);

        for (size_t i = 1; i < m_panels.size() && i <= 6; ++i)
        {
            size_t idx = i;
            hotkeys.registerHotkey(static_cast<ImGuiKey>(ImGuiKey_F1 + static_cast<int>(i) - 1),
                                   [this, idx] { m_executor.execute(TogglePanelCommand(&m_panels, idx)); });
        }

        std::filesystem::path  scene_file = m_bridge->getSceneFileFromConfig();
        std::shared_ptr<Scene> scene;
        if (std::filesystem::exists(scene_file))
        {
            RE_LOG_INFO("Auto-loading scene from: " + scene_file.string());
            scene = m_bridge->loadScene(scene_file.string());
            if (scene)
            {
                m_bridge->setCurrentScene(scene);
                RE_LOG_INFO("Scene loaded successfully.");
            }
            else
            {
                RE_LOG_WARN("Failed to load scene, creating default scene instead.");
                scene = m_bridge->createDefaultScene();
                m_bridge->setCurrentScene(scene);
            }
        }
        else
        {
            RE_LOG_INFO("No scene file found, creating default scene.");
            scene = m_bridge->createDefaultScene();
            m_bridge->setCurrentScene(scene);
        }

        m_bridge->initializeCameraForScene(scene);

        m_initialized = true;
    }

    void Editor::shutdown()
    {
        if (!m_initialized)
            return;

        m_panels.clear();
        m_bridge.reset();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_engine)
        {
            m_engine->shutdown();
            m_engine.reset();
        }

        m_initialized = false;
    }

    void Editor::run()
    {
        RE_LOG_INFO("<<< Run in Editor-Mode. >>>");

        while (!m_engine->getWindow().shouldClose())
            tick();
    }

    void Editor::tick()
    {
        m_engine->tick();

        beginFrame();
        m_context->getHotkeyManager().process();
        render();
        endFrame();

        m_engine->getWindow().swapBuffer();
    }

    void Editor::beginFrame() const
    {
        if (!m_initialized)
            return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Editor::render() const
    {
        if (!m_initialized)
            return;

        if (!m_panels.empty() && m_panels[0])
            m_panels[0]->render();

        for (size_t i = 1; i < m_panels.size(); ++i)
        {
            if (m_panels[i] && m_panels[i]->isOpen())
                m_panels[i]->render();
        }
    }

    void Editor::endFrame() const
    {
        if (!m_initialized)
            return;

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            void* backup = Window::getCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            Window::setCurrentContext(backup);
        }
    }

} // namespace RealmEngine
