#include "editor/editor.h"
#include <memory>

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

        m_initialized = true;
    }

    void Editor::shutdown()
    {
        if (!m_initialized)
            return;

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

        ImGui::Begin("Viewport");
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();

        int window_width  = g_context.m_window->getWidth();
        int window_height = g_context.m_window->getHeight();

        if (viewport_size.x > 0 && viewport_size.y > 0 && window_width > 0 && window_height > 0)
        {
            static GLuint viewport_texture = 0;
            static int    last_width = 0, last_height = 0;

            if (viewport_texture == 0 || window_width != last_width || window_height != last_height)
            {
                if (viewport_texture != 0)
                    glDeleteTextures(1, &viewport_texture);

                glGenTextures(1, &viewport_texture);
                glBindTexture(GL_TEXTURE_2D, viewport_texture);
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                last_width  = window_width;
                last_height = window_height;
            }

            // Copy from default framebuffer
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glReadBuffer(GL_BACK);
            glBindTexture(GL_TEXTURE_2D, viewport_texture);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, window_width, window_height, 0);

            ImGui::Image(static_cast<intptr_t>(viewport_texture), viewport_size, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();
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
