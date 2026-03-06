#include "panels/project_settings_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "functional/resource/config_manager.h"

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

        bool changed = false;
        changed |= ImGui::DragInt("Width", &copy.width, 1, 320, 7680);
        changed |= ImGui::DragInt("Height", &copy.height, 1, 240, 4320);
        char title_buf[256];
        strncpy(title_buf, copy.title.c_str(), sizeof(title_buf) - 1);
        title_buf[sizeof(title_buf) - 1] = '\0';
        if (ImGui::InputText("Title", title_buf, sizeof(title_buf)))
        {
            copy.title = title_buf;
            changed    = true;
        }
        changed |= ImGui::Checkbox("Fullscreen", &copy.fullscreen);
        changed |= ImGui::Checkbox("VSync", &copy.vsync);
        changed |= ImGui::DragInt("MSAA Samples", &copy.msaa_samples, 1, 0, 8);

        if (copy.msaa_samples != 0 && copy.msaa_samples != 1 && copy.msaa_samples != 2 && copy.msaa_samples != 4 &&
            copy.msaa_samples != 8)
        {
            copy.msaa_samples = 4;
            changed           = true;
        }

        if (changed)
            m_bridge->getConfig().setWindowConfig(copy);
    }

    void ProjectSettingsWidget::renderRendererSection()
    {
        const auto&    cfg  = m_bridge->getConfig().getRendererConfig();
        RendererConfig copy = cfg;

        bool changed = false;
        {
            static const char* s_pipeline_mode_labels[] = {"Forward", "Deferred"};
            int                pipeline_idx             = static_cast<int>(copy.pipeline_mode);
            if (ImGui::Combo("Pipeline Mode", &pipeline_idx, s_pipeline_mode_labels, 2))
            {
                copy.pipeline_mode = static_cast<PipelineMode>(pipeline_idx);
                changed            = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(requires restart)");
        }

        if (ImGui::TreeNode("Camera"))
        {
            changed |= ImGui::DragFloat("FOV", &copy.camera_fov, 1.0f, 1.0f, 179.0f);
            changed |= ImGui::DragFloat("Near Plane", &copy.camera_near_plane, 0.01f, 0.001f, 10.0f);
            changed |= ImGui::DragFloat("Far Plane", &copy.camera_far_plane, 10.0f, 10.0f, 10000.0f);
            if (ImGui::TreeNode("Initial Position"))
            {
                changed |= ImGui::DragFloat("Pos X", &copy.camera_initial_pos_x, 0.1f, -1000.0f, 1000.0f);
                changed |= ImGui::DragFloat("Pos Y", &copy.camera_initial_pos_y, 0.1f, -1000.0f, 1000.0f);
                changed |= ImGui::DragFloat("Pos Z", &copy.camera_initial_pos_z, 0.1f, -1000.0f, 1000.0f);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Initial Look At"))
            {
                changed |= ImGui::DragFloat("Look X", &copy.camera_look_at_x, 0.1f, -1000.0f, 1000.0f);
                changed |= ImGui::DragFloat("Look Y", &copy.camera_look_at_y, 0.1f, -1000.0f, 1000.0f);
                changed |= ImGui::DragFloat("Look Z", &copy.camera_look_at_z, 0.1f, -1000.0f, 1000.0f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Post-Processing"))
        {
            changed |= ImGui::Checkbox("GTAO", &copy.ao_enabled);
            changed |= ImGui::DragFloat("GTAO Radius", &copy.ao_radius, 0.01f, 0.01f, 2.0f);
            changed |= ImGui::DragFloat("GTAO Power", &copy.ao_power, 0.1f, 0.5f, 5.0f);
            changed |= ImGui::DragFloat("GTAO Intensity", &copy.ao_intensity, 0.05f, 0.0f, 1.0f);
            changed |= ImGui::DragInt("GTAO Num Directions", &copy.gtao_num_directions, 1, 4, 8);
            changed |= ImGui::DragInt("GTAO Num Steps", &copy.gtao_num_steps, 1, 4, 8);
            ImGui::Separator();
            changed |= ImGui::Checkbox("Bloom", &copy.bloom_enabled);
            changed |= ImGui::DragFloat("Bloom Intensity", &copy.bloom_intensity, 0.1f, 0.0f, 10.0f);
            changed |= ImGui::DragInt("Bloom Iterations", &copy.bloom_iterations, 1, 1, 32);
            {
                static const char* s_bloom_dir_labels[] = {"Both", "Horizontal", "Vertical"};
                int                dir_idx              = copy.bloom_direction;
                if (ImGui::Combo("Bloom Direction", &dir_idx, s_bloom_dir_labels, 3))
                {
                    copy.bloom_direction = dir_idx;
                    changed              = true;
                }
            }
            changed |= ImGui::DragFloat("Bloom Brightness Cutoff", &copy.bloom_brightness_cutoff, 0.1f, 0.0f, 10.0f);
            changed |= ImGui::Checkbox("Tone mapping", &copy.tonemapping_enabled);
            changed |= ImGui::DragFloat("Gamma", &copy.gamma_correction_factor, 0.1f, 1.0f, 3.0f);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Environment"))
        {
            char hdri_buf[512];
            strncpy(hdri_buf, copy.hdri_path.c_str(), sizeof(hdri_buf) - 1);
            hdri_buf[sizeof(hdri_buf) - 1] = '\0';
            if (ImGui::InputText("HDRI Path", hdri_buf, sizeof(hdri_buf)))
            {
                copy.hdri_path = hdri_buf;
                changed        = true;
            }
            changed |= ImGui::ColorEdit4("Clear Color", &copy.clear_color_r);
            ImGui::TreePop();
        }

        if (changed)
            m_bridge->getConfig().setRendererConfig(copy);
    }

    void ProjectSettingsWidget::renderInputSection()
    {
        const auto&    v_cfg  = m_bridge->getConfig().getViewportConfig();
        const auto&    e_cfg  = m_bridge->getConfig().getEngineConfig();
        ViewportConfig v_copy = v_cfg;
        EngineConfig   e_copy = e_cfg;

        bool changed = false;
        changed |= ImGui::DragFloat("Camera Move Speed", &v_copy.camera_move_speed, 0.1f, 0.1f, 100.0f);
        changed |= ImGui::DragFloat("Camera Sprint Multiplier", &v_copy.camera_sprint_multiplier, 0.1f, 1.0f, 10.0f);
        changed |= ImGui::DragFloat("Camera Mouse Sensitivity", &v_copy.camera_mouse_sensitivity, 0.01f, 0.01f, 2.0f);
        changed |= ImGui::DragFloat("Max Delta Time", &e_copy.max_delta_time, 0.01f, 0.01f, 1.0f);

        char scene_buf[256];
        strncpy(scene_buf, e_copy.scene_file.c_str(), sizeof(scene_buf) - 1);
        scene_buf[sizeof(scene_buf) - 1] = '\0';
        if (ImGui::InputText("Default Scene File", scene_buf, sizeof(scene_buf)))
        {
            e_copy.scene_file = scene_buf;
            changed           = true;
        }

        if (changed)
        {
            m_bridge->getConfig().setViewportConfig(v_copy);
            m_bridge->getConfig().setEngineConfig(e_copy);
        }
    }

    void ProjectSettingsWidget::renderPhysicsSection()
    {
        const auto&   cfg  = m_bridge->getConfig().getPhysicsConfig();
        PhysicsConfig copy = cfg;

        bool changed = false;
        changed |= ImGui::Checkbox("Physics Enabled", &copy.enabled);
        changed |= ImGui::DragFloat("Gravity", &copy.gravity, 0.1f, -50.0f, 0.0f);
        changed |= ImGui::DragInt("Max Substeps", &copy.max_substeps, 1, 1, 16);
        changed |= ImGui::DragFloat("Fixed Timestep", &copy.fixed_timestep, 0.001f, 0.001f, 0.1f, "%.3f");

        if (changed)
            m_bridge->getConfig().setPhysicsConfig(copy);
    }

} // namespace RealmEngine
