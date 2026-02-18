#include "module/ecs/systems/audio_system.h"

#include <miniaudio.h>

#include <filesystem>

#include "core/base/macros.h"
#include "module/ecs/components/audio/audio_source.h"
#include "module/ecs/components/transform.h"
#include "module/ecs/components/world_transform.h"
#include "module/resource/config_manager.h"
#include "module/scene/scene.h"

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
            engine_config.sampleRate = config.sample_rate;
        if (config.channels > 0)
            engine_config.channels = static_cast<ma_uint32>(static_cast<unsigned>(config.channels));

        ma_result result = ma_engine_init(&engine_config, raw);
        if (result != MA_SUCCESS)
        {
            delete raw;
            RE_LOG_ERROR("AudioSystem: failed to initialize miniaudio engine");
            return;
        }

        m_engine.reset(raw);
        ma_engine_set_volume(m_engine.get(), config.master_volume);
    }

    void AudioSystem::shutdown()
    {
        clearSceneSounds();
        m_engine.reset();
    }

    void AudioSystem::clearSceneSounds()
    {
        for (auto& [entity, sound] : m_entity_sounds)
            sound_deleter(sound);
        m_entity_sounds.clear();
    }

    void AudioSystem::tick(Scene* scene, float delta_time, const std::string& asset_folder)
    {
        (void)delta_time;
        if (!m_engine || !scene)
            return;

        auto& registry = scene->getRegistry();
        auto  view     = registry.view<AudioSource>();

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

            if (src.spatial)
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
    }

    void AudioSystem::setListener(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up)
    {
        if (!m_engine)
            return;

        ma_engine_listener_set_position(m_engine.get(), 0, pos.x, pos.y, pos.z);
        ma_engine_listener_set_direction(m_engine.get(), 0, forward.x, forward.y, forward.z);
        ma_engine_listener_set_world_up(m_engine.get(), 0, up.x, up.y, up.z);
    }

    void AudioSystem::playSound(const std::string& path, const std::string& asset_folder, bool loop, float volume)
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

        ma_result result = ma_engine_play_sound(m_engine.get(), full_path.c_str(), nullptr);
        if (result != MA_SUCCESS)
            RE_LOG_ERROR("AudioSystem: failed to play: " + full_path);

        (void)loop;
        (void)volume;
    }

    void AudioSystem::startSoundForEntity(Scene* scene, entt::entity entity, const std::string& asset_folder)
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

        ma_sound* sound  = new ma_sound;
        ma_uint32 flags  = src->spatial ? 0 : MA_SOUND_FLAG_NO_SPATIALIZATION;
        ma_result result = ma_sound_init_from_file(m_engine.get(), full_path.c_str(), flags, nullptr, nullptr, sound);
        if (result != MA_SUCCESS)
        {
            delete sound;
            RE_LOG_ERROR("AudioSystem: failed to load: " + full_path);
            return;
        }

        m_entity_sounds[entity] = sound;

        ma_sound_set_volume(sound, src->volume);
        ma_sound_set_looping(sound, src->loop ? MA_TRUE : MA_FALSE);

        if (src->spatial)
        {
            ma_sound_set_positioning(sound, ma_positioning_absolute);
            updateSoundPosition(scene, entity);
        }

        ma_sound_start(sound);
        src->playing = true;
    }

    void AudioSystem::stopSoundForEntity(entt::entity entity)
    {
        auto it = m_entity_sounds.find(entity);
        if (it != m_entity_sounds.end())
        {
            ma_sound_stop(it->second);
            sound_deleter(it->second);
            m_entity_sounds.erase(it);
        }
    }

    void AudioSystem::updateSoundPosition(Scene* scene, entt::entity entity)
    {
        auto it = m_entity_sounds.find(entity);
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
