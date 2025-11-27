#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include "editor/widget.h"

namespace RealmEngine
{
    class ViewportPanel : public Widget
    {
    public:
        ViewportPanel();
        ~ViewportPanel() override;

        ViewportPanel(const ViewportPanel&)            = delete;
        ViewportPanel& operator=(const ViewportPanel&) = delete;
        ViewportPanel(ViewportPanel&&)                 = default;
        ViewportPanel& operator=(ViewportPanel&&)      = default;

        void render() override;

    private:
        void updateTexture(int width, int height);

        GLuint m_viewport_texture {0};
        int    m_texture_width {0};
        int    m_texture_height {0};
    };

} // namespace RealmEngine
