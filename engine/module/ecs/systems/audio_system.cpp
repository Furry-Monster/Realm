#include "module/ecs/systems/audio_system.h"

#include <miniaudio.h>

#include <algorithm>
#include <filesystem>

#include "core/base/macros.h"
#include "module/ecs/components/audio/audio_source.h"
#include "functional/ecs/components/transform.h"
#include "functional/ecs/components/world_transform.h"
#include "functional/resource/config_manager.h"
#include "functional/scene/scene.h"

namespace RealmEngine
{
    namespace
    {
        void engine_deleter(ma_engine* p)
        {
            if (p)
            {
                ma_engine_uninit(p);
                delete p;
            }
        }

        void sound_deleter(ma_sound* p)
        {
            if (p)
            {
                ma_sound_uninit(p);
                delete p;
            }
        }
    } // namespace

    AudioSystem::AudioSystem() : m_engine(nullptr, &engine_deleter) {}

    AudioSystem::~AudioSystem() noexcept { shutdown(); }

    void AudioSystem::initialize(const AudioConfig& config)
    {
        if (!config.enabled)
            return;

        ma_engine*       raw           = new ma_engine;
        ma_engine_config engine_config = ma_engine_config_init();

        if (config.sample_rate > 0)
            engine_config.sampleRate = static_cast<ma_uint32>(config.sample_rate);
        if (config.channels > 0)
            engine_config.channels = static_cast<ma_uint32>(static_cast<unsigned>(config.channels));

        const ma_result result = ma_engine_init(&engine_config, raw);
        if (result != MA_SUCCESS)
        {
            delete raw;
            RE_LOG_ERROR("AudioSystem: failed to initialize miniaudio engine");
            return;
        }

        m_engine.reset(raw);
        ma_engine_set_volume(m_engine.get(), config.master_volume);
        m_spatial_enabled = config.spatial_enabled;
    }

    void AudioSystem::shutdown()
    {
        clearSceneSounds();
        for (ma_sound* s : m_one_shot_sounds)
            sound_deleter(s);
        m_one_shot_sounds.clear();
        m_engine.reset();
    }

    void AudioSystem::clearSceneSounds()
    {
        for (auto& [entity, sound] : m_entity_sounds)
            sound_deleter(sound);
        m_entity_sounds.clear();
    }

    void AudioSystem::pruneOneShotSounds()
    {
        m_one_shot_sounds.erase(std::remove_if(m_one_shot_sounds.begin(),
                                               m_one_shot_sounds.end(),
                                               [](ma_sound* s) {
                                                   if (ma_sound_is_playing(s) == MA_FALSE)
                                                   {
                                                       sound_deleter(s);
                                                       return true;
                                                   }
                                                   return false;
                                               }),
                                m_one_shot_sounds.end());
    }

    void AudioSystem::tick(Scene* scene, const float delta_time, const std::string& asset_folder)
    {
        (void)delta_time;
        if (!m_engine || !scene)
            return;

        auto&      registry = scene->getRegistry();
        const auto view     = registry.view<AudioSource>();

        for (auto entity : view)
        {
            auto& src = view.get<AudioSource>(entity);

            if (src.clip_path.empty())
                continue;

            auto it = m_entity_sounds.find(entity);
            if (it == m_entity_sounds.end())
            {
                if (src.play_on_start && !src.start_attempted)
                {
                    src.start_attempted = true;
                    startSoundForEntity(scene, entity, asset_folder);
                }
                continue;
            }

            ma_sound* sound = it->second;
            src.playing     = ma_sound_is_playing(sound) == MA_TRUE;

            if (m_spatial_enabled && src.spatial)
                updateSoundPosition(scene, entity);

            ma_sound_set_volume(sound, src.volume);
            ma_sound_set_looping(sound, src.loop ? MA_TRUE : MA_FALSE);
        }

        std::vector<entt::entity> to_remove;
        for (auto& [entity, sound] : m_entity_sounds)
        {
            if (!registry.valid(entity) || !registry.all_of<AudioSource>(entity))
                to_remove.push_back(entity);
            else
            {
                const auto* as = scene->tryGet<AudioSource>(entity);
                if (as && ma_sound_is_playing(sound) == MA_FALSE && !as->loop)
                    to_remove.push_back(entity);
            }
        }
        for (entt::entity e : to_remove)
        {
            auto it = m_entity_sounds.find(e);
            if (it != m_entity_sounds.end())
            {
                sound_deleter(it->second);
                m_entity_sounds.erase(it);
            }
        }
        pruneOneShotSounds();
    }

    void AudioSystem::setListener(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up)
    {
        if (!m_engine)
            return;

        ma_engine_listener_set_position(m_engine.get(), 0, pos.x, pos.y, pos.z);
        ma_engine_listener_set_direction(m_engine.get(), 0, forward.x, forward.y, forward.z);
        ma_engine_listener_set_world_up(m_engine.get(), 0, up.x, up.y, up.z);
    }

    void AudioSystem::playSound(const std::string& path,
                                const std::string& asset_folder,
                                const bool         loop,
                                const float        volume)
    {
        if (!m_engine)
            return;

        std::string full_path = path;
        if (!std::filesystem::path(path).is_absolute())
            full_path = (std::filesystem::path(asset_folder) / path).generic_string();

        if (!std::filesystem::exists(full_path))
        {
            RE_LOG_WARN("AudioSystem: file not found: " + full_path);
            return;
        }

        ma_sound*       sound  = new ma_sound;
        const ma_result result = ma_sound_init_from_file(
            m_engine.get(), full_path.c_str(), MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, nullptr, sound);
        if (result != MA_SUCCESS)
        {
            delete sound;
            RE_LOG_ERROR("AudioSystem: failed to play: " + full_path);
            return;
        }

        ma_sound_set_volume(sound, volume);
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
        ma_sound_start(sound);
        m_one_shot_sounds.push_back(sound);
    }

    void AudioSystem::startSoundForEntity(Scene* scene, const entt::entity entity, const std::string& asset_folder)
    {
        if (!m_engine || !scene)
            return;

        auto* src = scene->tryGet<AudioSource>(entity);
        if (!src || src->clip_path.empty())
            return;

        std::string full_path = src->clip_path;
        if (!std::filesystem::path(src->clip_path).is_absolute())
            full_path = (std::filesystem::path(asset_folder) / src->clip_path).generic_string();

        if (!std::filesystem::exists(full_path))
        {
            RE_LOG_WARN("AudioSystem: file not found: " + full_path);
            return;
        }

        ma_sound*       sound = new ma_sound;
        const ma_uint32 flags = (m_spatial_enabled && src->spatial) ? 0 : MA_SOUND_FLAG_NO_SPATIALIZATION;
        const ma_result result =
            ma_sound_init_from_file(m_engine.get(), full_path.c_str(), flags, nullptr, nullptr, sound);
        if (result != MA_SUCCESS)
        {
            delete sound;
            RE_LOG_ERROR("AudioSystem: failed to load: " + full_path);
            return;
        }

        m_entity_sounds[entity] = sound;

        ma_sound_set_volume(sound, src->volume);
        ma_sound_set_looping(sound, src->loop ? MA_TRUE : MA_FALSE);

        if (m_spatial_enabled && src->spatial)
        {
            ma_sound_set_positioning(sound, ma_positioning_absolute);
            updateSoundPosition(scene, entity);
        }

        ma_sound_start(sound);
        src->playing = true;
    }

    void AudioSystem::playSoundForEntity(Scene* scene, const entt::entity entity, const std::string& asset_folder)
    {
        const auto* src = scene->tryGet<AudioSource>(entity);
        if (!src || src->clip_path.empty())
            return;
        if (m_entity_sounds.count(entity))
            stopSoundForEntity(entity);
        startSoundForEntity(scene, entity, asset_folder);
    }

    void AudioSystem::stopSoundForEntity(const entt::entity entity)
    {
        const auto it = m_entity_sounds.find(entity);
        if (it != m_entity_sounds.end())
        {
            ma_sound_stop(it->second);
            sound_deleter(it->second);
            m_entity_sounds.erase(it);
        }
    }

    void AudioSystem::updateSoundPosition(Scene* scene, const entt::entity entity)
    {
        const auto it = m_entity_sounds.find(entity);
        if (it == m_entity_sounds.end())
            return;

        glm::vec3 pos {0.0f};
        if (const auto* wt = scene->tryGet<WorldTransform>(entity))
            pos = glm::vec3(wt->matrix[3]);
        else if (const auto* t = scene->tryGet<Transform>(entity))
            pos = t->position;

        ma_sound_set_position(it->second, pos.x, pos.y, pos.z);
    }

} // namespace RealmEngine
