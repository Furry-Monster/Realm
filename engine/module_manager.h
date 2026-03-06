#pragma once

#include <memory>

namespace RealmEngine
{
    class ConfigManager;
    class EventBus;
    class AudioSystem;

    class ModuleManager
    {
    public:
        ModuleManager();
        ~ModuleManager() noexcept;

        ModuleManager(const ModuleManager&)            = delete;
        ModuleManager(ModuleManager&&)                = delete;
        ModuleManager& operator=(const ModuleManager&) = delete;
        ModuleManager& operator=(ModuleManager&&)   = delete;

        void initialize(const ConfigManager& config, EventBus& event_bus);
        void shutdown();

        AudioSystem* getAudioSystem() const;


    private:
#if REALM_BUILD_AUDIO
        std::unique_ptr<AudioSystem> m_audio;
#endif
    };
} // namespace RealmEngine
