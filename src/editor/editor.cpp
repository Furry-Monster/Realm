#include "editor/editor.h"
#include <memory>

#include "editor/widgets/viewport_panel.h"
#include "engine.h"
#include "global_context.h"
#include "utils.h"
#include "window.h"

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

        m_panels.push_back(std::make_unique<ViewportPanel>());

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
        info("<<< Run in Editor-Mode. >>>");

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

        ImGui::DockSpaceOverViewport();

        for (const auto& panel : m_panels)
        {
            if (panel && panel->isOpen())
                panel->render();
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
