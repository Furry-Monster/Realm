#pragma once

#include <filesystem>
#include <string>

namespace RealmEngine
{
    enum class EditorTheme
    {
        Dark,
        Light,
        Classic
    };

    struct EditorPreferences
    {
        EditorTheme theme              = EditorTheme::Dark;
        float       font_scale         = 1.0f;
        bool        auto_save          = false;
        float       auto_save_interval = 300.0f;
    };

    class EditorPreferencesManager
    {
    public:
        static bool load(EditorPreferences& prefs, const std::filesystem::path& path);
        static bool save(const EditorPreferences& prefs, const std::filesystem::path& path);
    };

} // namespace RealmEngine
