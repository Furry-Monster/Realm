#include "viewport_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "renderer/viewport_display_mode.h"
#include "resource/config_manager.h"
#include "rhi/rhi_texture.h"

#include <imgui.h>

namespace RealmEngine
{
    static const char* displayModeLabels[] =
        {"Lit", "Albedo", "Normals", "Metallic", "Roughness", "Material AO", "Emissive", "SSAO", "Depth"};

    static const char* gbufferPreviewLabels[] =
        {"Final Output", "GBuffer: Albedo+AO", "GBuffer: Normal+Metallic", "GBuffer: Emissive+Roughness", "GBuffer: Depth"};

    ViewportWidget::ViewportWidget(EditorEngineBridge& bridge) : Widget("Viewport"), m_bridge(&bridge)
    {
        m_bridge->setRenderToViewportTexture(true);
    }

    static void drawTexturePreview(RHITexture* tex, ImVec2 avail)
    {
        if (!tex)
        {
            ImGui::TextDisabled("Texture not available");
            return;
        }

        float w = static_cast<float>(tex->getWidth());
        float h = static_cast<float>(tex->getHeight());

        if (w <= 0.0f || h <= 0.0f || avail.x <= 0.0f || avail.y <= 0.0f)
        {
            ImGui::TextDisabled("Viewport resizing...");
            return;
        }

        float  scale = std::min(avail.x / w, avail.y / h);
        ImVec2 display_size(w * scale, h * scale);

        ImTextureID tid = static_cast<ImTextureID>(static_cast<intptr_t>(tex->getNativeHandle()));
        ImGui::Image(tid, display_size, ImVec2(0, 1), ImVec2(1, 0));
    }

    void ViewportWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_name.c_str(), &m_open))
        {
            ImGui::End();
            return;
        }

        bool is_deferred = m_bridge->getPipelineMode() == PipelineMode::Deferred;

        // Pipeline mode label
        if (is_deferred)
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "[Deferred]");
        else
            ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "[Forward]");
        ImGui::SameLine();

        // Display mode combo
        float combo_width = ImGui::GetContentRegionAvail().x * 0.4f;
        ImGui::SetNextItemWidth(combo_width);
        int mode = static_cast<int>(m_bridge->getViewportDisplayMode());
        if (ImGui::Combo("##DisplayMode", &mode, displayModeLabels, static_cast<int>(ViewportDisplayMode::Count)))
        {
            m_bridge->setViewportDisplayMode(static_cast<ViewportDisplayMode>(mode));
        }

        // G-Buffer preview selector (deferred only)
        if (is_deferred)
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(combo_width);
            ImGui::Combo("##GBufferPreview", &m_gbuffer_preview, gbufferPreviewLabels, 5);
        }
        else
        {
            m_gbuffer_preview = 0;
        }

        ImVec2 avail = ImGui::GetContentRegionAvail();

        if (m_gbuffer_preview == 0)
        {
            drawTexturePreview(m_bridge->getViewportTexture(), avail);
        }
        else
        {
            RHITexture* gbuf_tex = nullptr;
            switch (m_gbuffer_preview)
            {
                case 1: gbuf_tex = m_bridge->getGBufferAlbedoAO(); break;
                case 2: gbuf_tex = m_bridge->getGBufferNormalMetallic(); break;
                case 3: gbuf_tex = m_bridge->getGBufferEmissiveRoughness(); break;
                case 4: gbuf_tex = m_bridge->getGBufferDepth(); break;
                default: break;
            }
            drawTexturePreview(gbuf_tex, avail);
        }

        ImGui::End();
    }

} // namespace RealmEngine
