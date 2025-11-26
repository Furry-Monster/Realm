#pragma once

#include <filesystem>
#include <string>

namespace RealmEngine
{
    struct GeneralConfig
    {
        std::filesystem::path root_folder;
        std::filesystem::path asset_folder;
        std::filesystem::path shader_folder;
    };

    struct WindowConfig
    {
        int         width        = 1280;
        int         height       = 720;
        std::string title        = "RealmEngine";
        bool        fullscreen   = false;
        bool        vsync        = true;
        int         msaa_samples = 4;
    };

    struct RendererConfig
    {
        float camera_fov           = 45.0f;
        float camera_near_plane    = 0.1f;
        float camera_far_plane     = 100.0f;
        float camera_initial_pos_x = 0.0f;
        float camera_initial_pos_y = 0.0f;
        float camera_initial_pos_z = 5.0f;
        float camera_look_at_x     = 0.0f;
        float camera_look_at_y     = 0.0f;
        float camera_look_at_z     = 0.0f;

        bool  bloom_enabled           = true;
        float bloom_intensity         = 1.0f;
        int   bloom_iterations        = 10;
        int   bloom_direction         = 0; // 0=BOTH, 1=HORIZONTAL, 2=VERTICAL
        float bloom_brightness_cutoff = 1.0f;
        bool  tonemapping_enabled     = true;
        float gamma_correction_factor = 2.2f;

        std::string hdri_path = "hdr/barcelona_rooftop.hdr";

        float clear_color_r = 0.0f;
        float clear_color_g = 0.0f;
        float clear_color_b = 0.0f;
        float clear_color_a = 1.0f;
    };

    struct GamePlayConfig
    {
        float camera_move_speed        = 5.0f;
        float camera_sprint_multiplier = 2.0f;
        float camera_mouse_sensitivity = 0.1f;

        std::string scene_file = "scene.json";

        float max_delta_time = 0.1f;
    };

    class ConfigManager
    {
    public:
        ConfigManager()           = default;
        ~ConfigManager() noexcept = default;

        ConfigManager(const ConfigManager&)                = delete;
        ConfigManager& operator=(const ConfigManager&)     = delete;
        ConfigManager(ConfigManager&&) noexcept            = default;
        ConfigManager& operator=(ConfigManager&&) noexcept = default;

        void initialize();
        void disposal();

        const GeneralConfig&  getGeneralConfig() const;
        const WindowConfig&   getWindowConfig() const;
        const RendererConfig& getRendererConfig() const;
        const GamePlayConfig& getGamePlayConfig() const;

        void setGeneralConfig(const GeneralConfig& config);
        void setWindowConfig(const WindowConfig& config);
        void setRendererConfig(const RendererConfig& config);
        void setGamePlayConfig(const GamePlayConfig& config);

        // Helper
        const std::filesystem::path& getRootFolder() const;
        const std::filesystem::path& getAssetFolder() const;
        const std::filesystem::path& getShaderFolder() const;

    private:
        GeneralConfig  m_general_config;
        WindowConfig   m_window_config;
        RendererConfig m_renderer_config;
        GamePlayConfig m_gameplay_config;
    };
} // namespace RealmEngine
