#include "viewport_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "renderer/viewport_display_mode.h"
#include "rhi/rhi_texture.h"

#include <imgui.h>

namespace RealmEngine
{
    static const char* displayModeLabels[] = {"Lit",
                                              "Albedo",
                                              "Normals",
                                              "Metallic",
                                              "Roughness",
                                              "Material AO",
                                              "Emissive",
                                              "SSAO",
                                              "Depth"};

    ViewportWidget::ViewportWidget(EditorEngineBridge& bridge) : Widget("Viewport"), m_bridge(&bridge)
    {
        m_bridge->setRenderToViewportTexture(true);
    }

    void ViewportWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_name.c_str(), &m_open))
        {
            ImGui::End();
            return;
        }

        int mode = static_cast<int>(m_bridge->getViewportDisplayMode());
        if (ImGui::Combo("Display Mode", &mode, displayModeLabels,
                         static_cast<int>(ViewportDisplayMode::Count)))
        {
            m_bridge->setViewportDisplayMode(static_cast<ViewportDisplayMode>(mode));
        }

        auto* tex = m_bridge->getViewportTexture();
        if (tex)
        {
            float  w  = static_cast<float>(tex->getWidth());
            float  h  = static_cast<float>(tex->getHeight());
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float  scale = std::min(avail.x / w, avail.y / h);
            ImVec2 display_size(w * scale, h * scale);

            ImTextureID tid = static_cast<ImTextureID>(static_cast<intptr_t>(tex->getNativeHandle()));
            ImGui::Image(tid, display_size, ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImGui::TextDisabled("Viewport texture not available");
        }

        ImGui::End();
    }

} // namespace RealmEngine
