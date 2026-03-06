#include "module_manager.h"

#include "core/event/event.h"
#include "core/event/event_bus.h"
#include "functional/resource/config_manager.h"

#if REALM_BUILD_AUDIO
#  include "module/audio/audio_system.h"
#endif

namespace RealmEngine
{
    ModuleManager::ModuleManager() = default;
    ModuleManager::~ModuleManager() noexcept = default;

    void ModuleManager::initialize(const ConfigManager& config, EventBus& event_bus)
    {
        (void)config;
        (void)event_bus;
#if REALM_BUILD_AUDIO
        m_audio = std::make_unique<AudioSystem>();
        m_audio->initialize(config.getAudioConfig());

        (void)event_bus.subscribe<SceneChangedEvent>([this](const SceneChangedEvent&) {
            if (m_audio)
                m_audio->clearSceneSounds();
        });
#endif
    }

    void ModuleManager::shutdown()
    {
#if REALM_BUILD_AUDIO
        if (m_audio)
        {
            m_audio->shutdown();
            m_audio.reset();
        }
#endif
    }

    AudioSystem* ModuleManager::getAudioSystem() const
    {
#if REALM_BUILD_AUDIO
        return m_audio.get();
#else
        return nullptr;
#endif
    }
} // namespace RealmEngine
