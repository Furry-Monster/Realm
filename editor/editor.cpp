#include "editor.h"

#include <filesystem>
#include <memory>

#include "core/log/log_macros.h"
#include "editor_context.h"
#include "engine.h"
#include "panels/file_dialog_widget.h"
#include "panels/menu_bar_widget.h"
#include "panels/properties_widget.h"
#include "panels/scene_hierarchy_widget.h"
#include "platform/input/input.h"
#include "platform/window/window.h"
#include "renderer/renderer.h"
#include "resource/config_manager.h"
#include "rhi/rhi_device.h"
#include "scene/components/camera_controller.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"

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
        (void)io;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        if (!m_engine)
        {
            m_engine = std::make_unique<Engine>();
            m_engine->boot();
        }

        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_engine->getWindow().getNativeHandle()), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        m_context = std::make_shared<EditorContext>();

        // Capture raw pointer for lambdas (Engine outlives file_dialog)
        Engine* engine = m_engine.get();

        auto file_dialog = std::make_shared<FileDialogWidget>();
        file_dialog->setOnFileSelected([file_dialog, engine](const std::filesystem::path& path) {
            SceneManager& scene_mgr = engine->getSceneManager();
            RHIDevice&    device    = engine->getRenderer().getDevice();
            if (file_dialog->getMode() == FileDialogWidget::Mode::Open)
            {
                auto loaded = scene_mgr.loadScene(path.string(), device);
                if (loaded)
                {
                    scene_mgr.setCurrentScene(loaded);

                    // Wire up camera controller for the loaded scene
                    const GamePlayConfig& gp = engine->getConfig().getGamePlayConfig();
                    loaded->getCameraController()->initialize(engine->getRenderer().getCamera(),
                                                              engine->getInput(),
                                                              gp.camera_mouse_sensitivity,
                                                              gp.camera_move_speed,
                                                              gp.camera_sprint_multiplier);

                    RE_LOG_INFO("Scene loaded from: " + path.string());
                }
                else
                {
                    RE_LOG_ERROR("Failed to load scene from: " + path.string());
                }
            }
            else // Save
            {
                if (scene_mgr.getCurrentScene())
                {
                    if (scene_mgr.saveCurrentScene(path.string()))
                        RE_LOG_INFO("Scene saved to: " + path.string());
                    else
                        RE_LOG_ERROR("Failed to save scene to: " + path.string());
                }
            }
        });

        m_panels.push_back(std::make_shared<MenuBarWidget>(*engine));
        m_panels.push_back(std::make_shared<SceneHierarchyWidget>(
            m_context, engine->getSceneManager(), engine->getEventBus()));
        m_panels.push_back(std::make_shared<PropertiesWidget>(m_context, engine->getSceneManager()));
        m_panels.push_back(file_dialog);

        auto widgets_shared = std::make_shared<std::vector<std::shared_ptr<Widget>>>(m_panels);
        auto menu_bar       = std::dynamic_pointer_cast<MenuBarWidget>(m_panels[0]);
        if (menu_bar)
        {
            menu_bar->setWidgets(widgets_shared);
            menu_bar->setFileDialog(file_dialog);
        }

        // Auto-load scene or create default
        ConfigManager& config    = m_engine->getConfig();
        SceneManager&  scene_mgr = m_engine->getSceneManager();
        RHIDevice&     device    = m_engine->getRenderer().getDevice();

        std::filesystem::path scene_file = config.getRootFolder() / config.getGamePlayConfig().scene_file;

        std::shared_ptr<Scene> scene;
        if (std::filesystem::exists(scene_file))
        {
            RE_LOG_INFO("Auto-loading scene from: " + scene_file.string());
            scene = scene_mgr.loadScene(scene_file.string(), device);
            if (scene)
            {
                scene_mgr.setCurrentScene(scene);
                RE_LOG_INFO("Scene loaded successfully.");
            }
            else
            {
                RE_LOG_WARN("Failed to load scene, creating default scene instead.");
                scene = scene_mgr.createDefaultScene(device);
                scene_mgr.setCurrentScene(scene);
            }
        }
        else
        {
            RE_LOG_INFO("No scene file found, creating default scene.");
            scene = scene_mgr.createDefaultScene(device);
            scene_mgr.setCurrentScene(scene);
        }

        // Initialize scene camera controller
        const GamePlayConfig& gp = config.getGamePlayConfig();
        scene->getCameraController()->initialize(m_engine->getRenderer().getCamera(),
                                                 m_engine->getInput(),
                                                 gp.camera_mouse_sensitivity,
                                                 gp.camera_move_speed,
                                                 gp.camera_sprint_multiplier);

        m_initialized = true;
    }

    void Editor::shutdown()
    {
        if (!m_initialized)
            return;

        m_panels.clear();

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
