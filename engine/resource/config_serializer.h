#pragma once

#include <json.hpp>
#include <string>

#include "resource/config_manager.h"

namespace RealmEngine
{
    class ConfigSerializer
    {
    public:
        ConfigSerializer()           = default;
        ~ConfigSerializer() noexcept = default;

        ConfigSerializer(const ConfigSerializer&)                = delete;
        ConfigSerializer& operator=(const ConfigSerializer&)     = delete;
        ConfigSerializer(ConfigSerializer&&) noexcept            = default;
        ConfigSerializer& operator=(ConfigSerializer&&) noexcept = default;

        static std::string serialize(const ConfigManager& config);
        static bool        deserialize(ConfigManager& config, const std::string& json_str);

        static bool saveToFile(const ConfigManager& config, const std::string& filepath, bool encrypt = false);
        static bool loadFromFile(ConfigManager& config, const std::string& filepath, bool encrypted = false);

    private:
        static void serializeConfig(const ConfigManager& config, nlohmann::json& json);
        static void deserializeConfig(ConfigManager& config, const nlohmann::json& json);

        static void serializeGeneralConfig(const GeneralConfig& general, nlohmann::json& json);
        static void serializeWindowConfig(const WindowConfig& window, nlohmann::json& json);
        static void serializeRendererConfig(const RendererConfig& renderer, nlohmann::json& json);
        static void serializeGamePlayConfig(const GamePlayConfig& gameplay, nlohmann::json& json);
        static void serializePhysicsConfig(const PhysicsConfig& physics, nlohmann::json& json);

        static void deserializeGeneralConfig(GeneralConfig& general, const nlohmann::json& json);
        static void deserializeWindowConfig(WindowConfig& window, const nlohmann::json& json);
        static void deserializeRendererConfig(RendererConfig& renderer, const nlohmann::json& json);
        static void deserializeGamePlayConfig(GamePlayConfig& gameplay, const nlohmann::json& json);
        static void deserializePhysicsConfig(PhysicsConfig& physics, const nlohmann::json& json);
    };

} // namespace RealmEngine
