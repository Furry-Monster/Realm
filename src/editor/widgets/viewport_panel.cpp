#include "editor/widgets/viewport_panel.h"

#include "global_context.h"
#include "window.h"

#include <imgui.h>

namespace RealmEngine
{
    ViewportPanel::ViewportPanel() : Widget("Viewport") {}

    ViewportPanel::~ViewportPanel()
    {
        if (m_viewport_texture != 0)
        {
            glDeleteTextures(1, &m_viewport_texture);
            m_viewport_texture = 0;
        }
    }

    void ViewportPanel::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        ImVec2 viewport_size = ImGui::GetContentRegionAvail();

        int window_width  = g_context.m_window->getWidth();
        int window_height = g_context.m_window->getHeight();

        if (viewport_size.x > 0 && viewport_size.y > 0 && window_width > 0 && window_height > 0)
        {
            if (m_viewport_texture == 0 || window_width != m_texture_width || window_height != m_texture_height)
                updateTexture(window_width, window_height);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glReadBuffer(GL_BACK);
            glBindTexture(GL_TEXTURE_2D, m_viewport_texture);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, window_width, window_height, 0);

            ImGui::Image(static_cast<intptr_t>(m_viewport_texture), viewport_size, ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::End();
    }

    void ViewportPanel::updateTexture(int width, int height)
    {
        if (m_viewport_texture != 0)
            glDeleteTextures(1, &m_viewport_texture);

        glGenTextures(1, &m_viewport_texture);
        glBindTexture(GL_TEXTURE_2D, m_viewport_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        m_texture_width  = width;
        m_texture_height = height;
    }

} // namespace RealmEngine
