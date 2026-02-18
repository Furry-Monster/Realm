#include "resource/config_serializer.h"

#include "core/base/macros.h"
#include "core/base/utils.h"
#include "resource/config_manager.h"

#include <fstream>
#include <sstream>

namespace RealmEngine
{
    std::string ConfigSerializer::serialize(const ConfigManager& config)
    {
        nlohmann::json json;
        serializeConfig(config, json);
        return json.dump(2);
    }

    bool ConfigSerializer::deserialize(ConfigManager& config, const std::string& json_str)
    {
        if (json_str.empty())
            return false;

        try
        {
            nlohmann::json json = nlohmann::json::parse(json_str);
            deserializeConfig(config, json);
            return true;
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to deserialize config: " + std::string(e.what()));
            return false;
        }
    }

    bool ConfigSerializer::saveToFile(const ConfigManager& config, const std::string& filepath, bool encrypt)
    {
        try
        {
            std::string json_str = serialize(config);
            std::string output   = json_str;

            if (encrypt)
            {
                std::string encrypted = xorEncrypt(json_str, DEFAULT_ENCRYPTION_KEY);
                output                = base64Encode(encrypted);
            }

            std::ofstream file(filepath);
            if (!file.is_open())
            {
                RE_LOG_ERROR("Failed to open config file for writing: " + filepath);
                return false;
            }

            file << output;
            file.close();
            return true;
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to save config to file: " + std::string(e.what()));
            return false;
        }
    }

    bool ConfigSerializer::loadFromFile(ConfigManager& config, const std::string& filepath, bool encrypted)
    {
        try
        {
            std::ifstream file(filepath);
            if (!file.is_open())
            {
                RE_LOG_WARN("Config file not found: " + filepath + ", using default config.");
                return false;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            std::string content  = buffer.str();
            std::string json_str = content;

            if (encrypted)
            {
                std::string decoded = base64Decode(content);
                json_str            = xorDecrypt(decoded, DEFAULT_ENCRYPTION_KEY);
            }

            return deserialize(config, json_str);
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to load config from file: " + std::string(e.what()));
            return false;
        }
    }

    void ConfigSerializer::serializeConfig(const ConfigManager& config, nlohmann::json& json)
    {
        json["version"] = 1;

        serializeGeneralConfig(config.getGeneralConfig(), json["general"]);
        serializeWindowConfig(config.getWindowConfig(), json["window"]);
        serializeRendererConfig(config.getRendererConfig(), json["renderer"]);
        serializeGamePlayConfig(config.getGamePlayConfig(), json["gameplay"]);
        serializePhysicsConfig(config.getPhysicsConfig(), json["physics"]);
    }

    void ConfigSerializer::deserializeConfig(ConfigManager& config, const nlohmann::json& json)
    {
        if (json.contains("general"))
        {
            GeneralConfig general = config.getGeneralConfig();
            deserializeGeneralConfig(general, json["general"]);
            config.setGeneralConfig(general);
        }

        if (json.contains("window"))
        {
            WindowConfig window = config.getWindowConfig();
            deserializeWindowConfig(window, json["window"]);
            config.setWindowConfig(window);
        }

        if (json.contains("renderer"))
        {
            RendererConfig renderer = config.getRendererConfig();
            deserializeRendererConfig(renderer, json["renderer"]);
            config.setRendererConfig(renderer);
        }

        if (json.contains("gameplay"))
        {
            GamePlayConfig gameplay = config.getGamePlayConfig();
            deserializeGamePlayConfig(gameplay, json["gameplay"]);
            config.setGamePlayConfig(gameplay);
        }

        if (json.contains("physics"))
        {
            PhysicsConfig physics = config.getPhysicsConfig();
            deserializePhysicsConfig(physics, json["physics"]);
            config.setPhysicsConfig(physics);
        }
    }

    void ConfigSerializer::serializeGeneralConfig(const GeneralConfig& general, nlohmann::json& json)
    {
        json["root_folder"]   = general.root_folder.generic_string();
        json["asset_folder"]  = general.asset_folder.generic_string();
        json["shader_folder"] = general.shader_folder.generic_string();
    }

    void ConfigSerializer::serializeWindowConfig(const WindowConfig& window, nlohmann::json& json)
    {
        json["width"]        = window.width;
        json["height"]       = window.height;
        json["title"]        = window.title;
        json["fullscreen"]   = window.fullscreen;
        json["vsync"]        = window.vsync;
        json["msaa_samples"] = window.msaa_samples;
    }

    void ConfigSerializer::serializeRendererConfig(const RendererConfig& renderer, nlohmann::json& json)
    {
        json["pipeline_mode"]        = static_cast<int>(renderer.pipeline_mode);
        json["camera_fov"]           = renderer.camera_fov;
        json["camera_near_plane"]    = renderer.camera_near_plane;
        json["camera_far_plane"]     = renderer.camera_far_plane;
        json["camera_initial_pos_x"] = renderer.camera_initial_pos_x;
        json["camera_initial_pos_y"] = renderer.camera_initial_pos_y;
        json["camera_initial_pos_z"] = renderer.camera_initial_pos_z;
        json["camera_look_at_x"]     = renderer.camera_look_at_x;
        json["camera_look_at_y"]     = renderer.camera_look_at_y;
        json["camera_look_at_z"]     = renderer.camera_look_at_z;
        json["ao_enabled"]           = renderer.ao_enabled;
        json["ao_radius"]            = renderer.ao_radius;
        json["ao_power"]             = renderer.ao_power;
        json["ao_intensity"]         = renderer.ao_intensity;
        json["gtao_num_directions"]  = renderer.gtao_num_directions;
        json["gtao_num_steps"]       = renderer.gtao_num_steps;

        json["bloom_enabled"]           = renderer.bloom_enabled;
        json["bloom_intensity"]         = renderer.bloom_intensity;
        json["bloom_iterations"]        = renderer.bloom_iterations;
        json["bloom_direction"]         = renderer.bloom_direction;
        json["bloom_brightness_cutoff"] = renderer.bloom_brightness_cutoff;
        json["tonemapping_enabled"]     = renderer.tonemapping_enabled;
        json["gamma_correction_factor"] = renderer.gamma_correction_factor;
        json["hdri_path"]               = renderer.hdri_path;
        json["clear_color_r"]           = renderer.clear_color_r;
        json["clear_color_g"]           = renderer.clear_color_g;
        json["clear_color_b"]           = renderer.clear_color_b;
        json["clear_color_a"]           = renderer.clear_color_a;
    }

    void ConfigSerializer::serializeGamePlayConfig(const GamePlayConfig& gameplay, nlohmann::json& json)
    {
        json["camera_move_speed"]        = gameplay.camera_move_speed;
        json["camera_sprint_multiplier"] = gameplay.camera_sprint_multiplier;
        json["camera_mouse_sensitivity"] = gameplay.camera_mouse_sensitivity;
        json["scene_file"]               = gameplay.scene_file;
        json["max_delta_time"]           = gameplay.max_delta_time;
    }

    void ConfigSerializer::deserializeGeneralConfig(GeneralConfig& general, const nlohmann::json& json)
    {
        if (json.contains("root_folder"))
            general.root_folder = json["root_folder"].get<std::string>();
        if (json.contains("asset_folder"))
            general.asset_folder = json["asset_folder"].get<std::string>();
        if (json.contains("shader_folder"))
            general.shader_folder = json["shader_folder"].get<std::string>();
    }

    void ConfigSerializer::deserializeWindowConfig(WindowConfig& window, const nlohmann::json& json)
    {
        if (json.contains("width"))
            window.width = json["width"].get<int>();
        if (json.contains("height"))
            window.height = json["height"].get<int>();
        if (json.contains("title"))
            window.title = json["title"].get<std::string>();
        if (json.contains("fullscreen"))
            window.fullscreen = json["fullscreen"].get<bool>();
        if (json.contains("vsync"))
            window.vsync = json["vsync"].get<bool>();
        if (json.contains("msaa_samples"))
            window.msaa_samples = json["msaa_samples"].get<int>();
    }

    void ConfigSerializer::deserializeRendererConfig(RendererConfig& renderer, const nlohmann::json& json)
    {
        if (json.contains("pipeline_mode"))
            renderer.pipeline_mode = static_cast<PipelineMode>(json["pipeline_mode"].get<int>());
        if (json.contains("camera_fov"))
            renderer.camera_fov = json["camera_fov"].get<float>();
        if (json.contains("camera_near_plane"))
            renderer.camera_near_plane = json["camera_near_plane"].get<float>();
        if (json.contains("camera_far_plane"))
            renderer.camera_far_plane = json["camera_far_plane"].get<float>();
        if (json.contains("camera_initial_pos_x"))
            renderer.camera_initial_pos_x = json["camera_initial_pos_x"].get<float>();
        if (json.contains("camera_initial_pos_y"))
            renderer.camera_initial_pos_y = json["camera_initial_pos_y"].get<float>();
        if (json.contains("camera_initial_pos_z"))
            renderer.camera_initial_pos_z = json["camera_initial_pos_z"].get<float>();
        if (json.contains("camera_look_at_x"))
            renderer.camera_look_at_x = json["camera_look_at_x"].get<float>();
        if (json.contains("camera_look_at_y"))
            renderer.camera_look_at_y = json["camera_look_at_y"].get<float>();
        if (json.contains("camera_look_at_z"))
            renderer.camera_look_at_z = json["camera_look_at_z"].get<float>();
        if (json.contains("ao_enabled"))
            renderer.ao_enabled = json["ao_enabled"].get<bool>();
        else if (json.contains("ssao_enabled"))
            renderer.ao_enabled = json["ssao_enabled"].get<bool>();
        if (json.contains("ao_radius"))
            renderer.ao_radius = json["ao_radius"].get<float>();
        else if (json.contains("ssao_radius"))
            renderer.ao_radius = json["ssao_radius"].get<float>();
        if (json.contains("ao_power"))
            renderer.ao_power = json["ao_power"].get<float>();
        else if (json.contains("ssao_power"))
            renderer.ao_power = json["ssao_power"].get<float>();
        if (json.contains("ao_intensity"))
            renderer.ao_intensity = json["ao_intensity"].get<float>();
        else if (json.contains("ssao_intensity"))
            renderer.ao_intensity = json["ssao_intensity"].get<float>();
        else if (json.contains("gtao_intensity"))
            renderer.ao_intensity = json["gtao_intensity"].get<float>();
        if (json.contains("gtao_num_directions"))
            renderer.gtao_num_directions = json["gtao_num_directions"].get<int>();
        if (json.contains("gtao_num_steps"))
            renderer.gtao_num_steps = json["gtao_num_steps"].get<int>();

        if (json.contains("bloom_enabled"))
            renderer.bloom_enabled = json["bloom_enabled"].get<bool>();
        if (json.contains("bloom_intensity"))
            renderer.bloom_intensity = json["bloom_intensity"].get<float>();
        if (json.contains("bloom_iterations"))
            renderer.bloom_iterations = json["bloom_iterations"].get<int>();
        if (json.contains("bloom_direction"))
            renderer.bloom_direction = json["bloom_direction"].get<int>();
        if (json.contains("bloom_brightness_cutoff"))
            renderer.bloom_brightness_cutoff = json["bloom_brightness_cutoff"].get<float>();
        if (json.contains("tonemapping_enabled"))
            renderer.tonemapping_enabled = json["tonemapping_enabled"].get<bool>();
        if (json.contains("gamma_correction_factor"))
            renderer.gamma_correction_factor = json["gamma_correction_factor"].get<float>();
        if (json.contains("hdri_path"))
            renderer.hdri_path = json["hdri_path"].get<std::string>();
        if (json.contains("clear_color_r"))
            renderer.clear_color_r = json["clear_color_r"].get<float>();
        if (json.contains("clear_color_g"))
            renderer.clear_color_g = json["clear_color_g"].get<float>();
        if (json.contains("clear_color_b"))
            renderer.clear_color_b = json["clear_color_b"].get<float>();
        if (json.contains("clear_color_a"))
            renderer.clear_color_a = json["clear_color_a"].get<float>();
    }

    void ConfigSerializer::deserializeGamePlayConfig(GamePlayConfig& gameplay, const nlohmann::json& json)
    {
        if (json.contains("camera_move_speed"))
            gameplay.camera_move_speed = json["camera_move_speed"].get<float>();
        if (json.contains("camera_sprint_multiplier"))
            gameplay.camera_sprint_multiplier = json["camera_sprint_multiplier"].get<float>();
        if (json.contains("camera_mouse_sensitivity"))
            gameplay.camera_mouse_sensitivity = json["camera_mouse_sensitivity"].get<float>();
        if (json.contains("scene_file"))
            gameplay.scene_file = json["scene_file"].get<std::string>();
        if (json.contains("max_delta_time"))
            gameplay.max_delta_time = json["max_delta_time"].get<float>();
    }

    void ConfigSerializer::serializePhysicsConfig(const PhysicsConfig& physics, nlohmann::json& json)
    {
        json["enabled"]        = physics.enabled;
        json["gravity"]        = physics.gravity;
        json["max_substeps"]   = physics.max_substeps;
        json["fixed_timestep"] = physics.fixed_timestep;
    }

    void ConfigSerializer::deserializePhysicsConfig(PhysicsConfig& physics, const nlohmann::json& json)
    {
        if (json.contains("enabled"))
            physics.enabled = json["enabled"].get<bool>();
        if (json.contains("gravity"))
            physics.gravity = json["gravity"].get<float>();
        if (json.contains("max_substeps"))
            physics.max_substeps = json["max_substeps"].get<int>();
        if (json.contains("fixed_timestep"))
            physics.fixed_timestep = json["fixed_timestep"].get<float>();
    }

} // namespace RealmEngine
