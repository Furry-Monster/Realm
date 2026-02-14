#include "editor_preferences.h"

#include "core/log/log_macros.h"
#include "platform/filesystem/filesystem.h"

#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <sstream>

namespace RealmEngine
{
    static const char* themeToString(EditorTheme t)
    {
        switch (t)
        {
            case EditorTheme::Dark:
                return "dark";
            case EditorTheme::Light:
                return "light";
            case EditorTheme::Classic:
                return "classic";
        }
        return "dark";
    }

    static EditorTheme stringToTheme(const std::string& s)
    {
        if (s == "light")
            return EditorTheme::Light;
        if (s == "classic")
            return EditorTheme::Classic;
        return EditorTheme::Dark;
    }

    bool EditorPreferencesManager::load(EditorPreferences& prefs, const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path))
        {
            if (save(prefs, path))
                RE_LOG_INFO("Created default editor preferences: " + path.string());
            return true;
        }
        auto content = FileSystem::readTextFile(path);
        if (!content || content->empty())
            return false;
        try
        {
            auto j = nlohmann::json::parse(*content);
            if (j.contains("theme"))
                prefs.theme = stringToTheme(j["theme"].get<std::string>());
            if (j.contains("font_scale"))
                prefs.font_scale = j["font_scale"].get<float>();
            if (j.contains("auto_save"))
                prefs.auto_save = j["auto_save"].get<bool>();
            if (j.contains("auto_save_interval"))
                prefs.auto_save_interval = j["auto_save_interval"].get<float>();
            return true;
        }
        catch (const std::exception& e)
        {
            RE_LOG_WARN("Failed to load editor preferences: " + std::string(e.what()));
            return false;
        }
    }

    bool EditorPreferencesManager::save(const EditorPreferences& prefs, const std::filesystem::path& path)
    {
        try
        {
            nlohmann::json j;
            j["theme"]              = themeToString(prefs.theme);
            j["font_scale"]         = prefs.font_scale;
            j["auto_save"]          = prefs.auto_save;
            j["auto_save_interval"] = prefs.auto_save_interval;
            return FileSystem::writeTextFile(path, j.dump(2));
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to save editor preferences: " + std::string(e.what()));
            return false;
        }
    }

} // namespace RealmEngine
