#pragma once

#include <functional>

#include "preferences/editor_preferences.h"
#include "widget.h"

namespace RealmEngine
{
    class PreferencesWidget : public Widget
    {
    public:
        explicit PreferencesWidget(EditorPreferences&                     prefs,
                                   std::function<void()>                  on_apply,
                                   std::function<std::filesystem::path()> get_prefs_path);
        ~PreferencesWidget() noexcept override = default;

        PreferencesWidget(const PreferencesWidget&)            = delete;
        PreferencesWidget& operator=(const PreferencesWidget&) = delete;
        PreferencesWidget(PreferencesWidget&&)                 = delete;
        PreferencesWidget& operator=(PreferencesWidget&&)      = delete;

        void render() override;

    private:
        void applyTheme(EditorTheme theme);

        EditorPreferences*                     m_prefs;
        std::function<void()>                  m_on_apply;
        std::function<std::filesystem::path()> m_get_prefs_path;
    };

} // namespace RealmEngine
