#include "panels/preferences_widget.h"

#include "preferences/editor_preferences.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace RealmEngine
{
    PreferencesWidget::PreferencesWidget(EditorPreferences&                     prefs,
                                         std::function<void()>                  on_apply,
                                         std::function<std::filesystem::path()> get_prefs_path) :
        Widget("Preferences"), m_prefs(&prefs), m_on_apply(std::move(on_apply)),
        m_get_prefs_path(std::move(get_prefs_path))
    {}

    void PreferencesWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        EditorPreferences copy = *m_prefs;

        if (ImGui::CollapsingHeader("Theme", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int         theme_idx = static_cast<int>(copy.theme);
            const char* themes[]  = {"Dark", "Light", "Classic"};
            if (ImGui::Combo("Theme", &theme_idx, themes, 3))
            {
                copy.theme = static_cast<EditorTheme>(theme_idx);
                applyTheme(copy.theme);
            }
        }

        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat("Font Scale", &copy.font_scale, 0.05f, 0.5f, 2.0f, "%.2f");
        }

        if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Auto Save", &copy.auto_save);
            if (ImGui::DragFloat(
                    "Auto Save Interval (seconds)", &copy.auto_save_interval, 10.0f, 60.0f, 3600.0f, "%.0f"))
            {
                if (copy.auto_save_interval < 10.0f)
                    copy.auto_save_interval = 10.0f;
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Apply"))
        {
            *m_prefs = copy;
            if (m_on_apply)
                m_on_apply();
            if (m_get_prefs_path)
            {
                auto path = m_get_prefs_path();
                if (!path.empty())
                    EditorPreferencesManager::save(*m_prefs, path);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Layout"))
        {
            ImGui::ClearIniSettings();
        }

        ImGui::End();
    }

    void PreferencesWidget::applyTheme(EditorTheme theme)
    {
        switch (theme)
        {
            case EditorTheme::Dark:
                ImGui::StyleColorsDark();
                break;
            case EditorTheme::Light:
                ImGui::StyleColorsLight();
                break;
            case EditorTheme::Classic:
                ImGui::StyleColorsClassic();
                break;
        }
    }

} // namespace RealmEngine
