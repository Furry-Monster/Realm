#include "editor.h"

#include <filesystem>
#include <memory>

#include "core/log/log_macros.h"
#include "editor_context.h"
#include "engine.h"
#include "global_context.h"
#include "panels/file_dialog_widget.h"
#include "panels/menu_bar_widget.h"
#include "panels/properties_widget.h"
#include "panels/scene_hierarchy_widget.h"
#include "platform/window/window.h"
#include "resource/config_manager.h"
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

        ImGui_ImplGlfw_InitForOpenGL(g_context.m_window->getGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        m_context = std::make_shared<EditorContext>();

        auto file_dialog = std::make_shared<FileDialogWidget>();
        file_dialog->setOnFileSelected([file_dialog](const std::filesystem::path& path) {
            if (file_dialog->getMode() == FileDialogWidget::Mode::Open)
            {
                auto loaded = g_context.m_scene->loadScene(path.string());
                if (loaded)
                {
                    g_context.m_scene->setCurrentScene(loaded);
                    RE_LOG_INFO("Scene loaded from: " + path.string());
                }
                else
                {
                    RE_LOG_ERROR("Failed to load scene from: " + path.string());
                }
            }
            else // Save
            {
                if (g_context.m_scene->getCurrentScene())
                {
                    if (g_context.m_scene->saveCurrentScene(path.string()))
                    {
                        RE_LOG_INFO("Scene saved to: " + path.string());
                    }
                    else
                    {
                        RE_LOG_ERROR("Failed to save scene to: " + path.string());
                    }
                }
            }
        });

        m_panels.push_back(std::make_shared<MenuBarWidget>());
        m_panels.push_back(std::make_shared<SceneHierarchyWidget>(m_context));
        m_panels.push_back(std::make_shared<PropertiesWidget>(m_context));
        m_panels.push_back(file_dialog);

        auto widgets_shared = std::make_shared<std::vector<std::shared_ptr<Widget>>>(m_panels);
        auto menu_bar       = std::dynamic_pointer_cast<MenuBarWidget>(m_panels[0]);
        if (menu_bar)
        {
            menu_bar->setWidgets(widgets_shared);
            menu_bar->setFileDialog(file_dialog);
        }

        // Auto-load scene.json if it exists
        std::filesystem::path scene_file =
            g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

        if (std::filesystem::exists(scene_file))
        {
            RE_LOG_INFO("Auto-loading scene from: " + scene_file.string());
            auto loaded = g_context.m_scene->loadScene(scene_file.string());
            if (loaded)
            {
                g_context.m_scene->setCurrentScene(loaded);
                RE_LOG_INFO("Scene loaded successfully.");
            }
            else
            {
                RE_LOG_WARN("Failed to load scene, creating default scene instead.");
                auto default_scene = g_context.m_scene->createDefaultScene();
                g_context.m_scene->setCurrentScene(default_scene);
            }
        }
        else
        {
            RE_LOG_INFO("No scene file found, creating default scene.");
            auto default_scene = g_context.m_scene->createDefaultScene();
            g_context.m_scene->setCurrentScene(default_scene);
        }

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
            m_engine->terminate();
            m_engine.reset();
        }

        m_initialized = false;
    }

    void Editor::run()
    {
        RE_LOG_INFO("<<< Run in Editor-Mode. >>>");

        while (!g_context.m_window->shouldClose())
            tick();
    }

    void Editor::tick()
    {
        m_engine->tick();

        beginFrame();
        render();
        endFrame();

        g_context.m_window->swapBuffer();
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
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

} // namespace RealmEngine
