#include "panels/project_settings_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "resource/config_manager.h"

#include <imgui.h>

namespace RealmEngine
{
    ProjectSettingsWidget::ProjectSettingsWidget(EditorEngineBridge& bridge) :
        Widget("Project Settings"), m_bridge(&bridge)
    {}

    void ProjectSettingsWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        if (ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen))
            renderWindowSection();
        if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
            renderRendererSection();
        if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen))
            renderInputSection();
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen))
            renderPhysicsSection();

        ImGui::Separator();
        if (ImGui::Button("Save"))
        {
            m_bridge->saveConfig();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(Some changes require restart)");

        ImGui::End();
    }

    void ProjectSettingsWidget::renderWindowSection()
    {
        const auto&  cfg  = m_bridge->getConfig().getWindowConfig();
        WindowConfig copy = cfg;

        ImGui::DragInt("Width", &copy.width, 1, 320, 7680);
        ImGui::DragInt("Height", &copy.height, 1, 240, 4320);
        char title_buf[256];
        strncpy(title_buf, copy.title.c_str(), sizeof(title_buf) - 1);
        title_buf[sizeof(title_buf) - 1] = '\0';
        if (ImGui::InputText("Title", title_buf, sizeof(title_buf)))
            copy.title = title_buf;
        ImGui::Checkbox("Fullscreen", &copy.fullscreen);
        ImGui::Checkbox("VSync", &copy.vsync);
        ImGui::DragInt("MSAA Samples", &copy.msaa_samples, 1, 0, 8);

        if (copy.msaa_samples != 0 && copy.msaa_samples != 1 && copy.msaa_samples != 2 && copy.msaa_samples != 4 &&
            copy.msaa_samples != 8)
        {
            copy.msaa_samples = 4;
        }

        m_bridge->getConfig().setWindowConfig(copy);
    }

    void ProjectSettingsWidget::renderRendererSection()
    {
        const auto&    cfg  = m_bridge->getConfig().getRendererConfig();
        RendererConfig copy = cfg;

        if (ImGui::TreeNode("Camera"))
        {
            ImGui::DragFloat("FOV", &copy.camera_fov, 1.0f, 1.0f, 179.0f);
            ImGui::DragFloat("Near Plane", &copy.camera_near_plane, 0.01f, 0.001f, 10.0f);
            ImGui::DragFloat("Far Plane", &copy.camera_far_plane, 10.0f, 10.0f, 10000.0f);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Post-Processing"))
        {
            ImGui::Checkbox("Bloom", &copy.bloom_enabled);
            ImGui::DragFloat("Bloom Intensity", &copy.bloom_intensity, 0.1f, 0.0f, 10.0f);
            ImGui::DragInt("Bloom Iterations", &copy.bloom_iterations, 1, 1, 32);
            ImGui::DragFloat("Bloom Brightness Cutoff", &copy.bloom_brightness_cutoff, 0.1f, 0.0f, 10.0f);
            ImGui::Checkbox("Tonemapping", &copy.tonemapping_enabled);
            ImGui::DragFloat("Gamma", &copy.gamma_correction_factor, 0.1f, 1.0f, 3.0f);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Environment"))
        {
            char hdri_buf[512];
            strncpy(hdri_buf, copy.hdri_path.c_str(), sizeof(hdri_buf) - 1);
            hdri_buf[sizeof(hdri_buf) - 1] = '\0';
            if (ImGui::InputText("HDRI Path", hdri_buf, sizeof(hdri_buf)))
                copy.hdri_path = hdri_buf;
            ImGui::ColorEdit4("Clear Color", &copy.clear_color_r);
            ImGui::TreePop();
        }

        m_bridge->getConfig().setRendererConfig(copy);
    }

    void ProjectSettingsWidget::renderInputSection()
    {
        const auto&    cfg  = m_bridge->getConfig().getGamePlayConfig();
        GamePlayConfig copy = cfg;

        ImGui::DragFloat("Camera Move Speed", &copy.camera_move_speed, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Camera Sprint Multiplier", &copy.camera_sprint_multiplier, 0.1f, 1.0f, 10.0f);
        ImGui::DragFloat("Camera Mouse Sensitivity", &copy.camera_mouse_sensitivity, 0.01f, 0.01f, 2.0f);
        ImGui::DragFloat("Max Delta Time", &copy.max_delta_time, 0.01f, 0.01f, 1.0f);

        char scene_buf[256];
        strncpy(scene_buf, copy.scene_file.c_str(), sizeof(scene_buf) - 1);
        scene_buf[sizeof(scene_buf) - 1] = '\0';
        if (ImGui::InputText("Default Scene File", scene_buf, sizeof(scene_buf)))
            copy.scene_file = scene_buf;

        m_bridge->getConfig().setGamePlayConfig(copy);
    }

    void ProjectSettingsWidget::renderPhysicsSection()
    {
        const auto&   cfg  = m_bridge->getConfig().getPhysicsConfig();
        PhysicsConfig copy = cfg;

        ImGui::Checkbox("Physics Enabled", &copy.enabled);
        ImGui::DragFloat("Gravity", &copy.gravity, 0.1f, -50.0f, 0.0f);
        ImGui::DragInt("Max Substeps", &copy.max_substeps, 1, 1, 16);
        ImGui::DragFloat("Fixed Timestep", &copy.fixed_timestep, 0.001f, 0.001f, 0.1f, "%.3f");

        m_bridge->getConfig().setPhysicsConfig(copy);
    }

} // namespace RealmEngine
