#pragma once

#include <entt/entity/entity.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

struct ma_engine;
struct ma_sound;

namespace RealmEngine
{
    struct AudioConfig;
    class Scene;

    class AudioSystem
    {
    public:
        AudioSystem();
        ~AudioSystem() noexcept;

        AudioSystem(const AudioSystem&)            = delete;
        AudioSystem& operator=(const AudioSystem&) = delete;

        void initialize(const AudioConfig& config);
        void shutdown();

        void tick(Scene* scene, float delta_time, const std::string& asset_folder);

        void setListener(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up);

        void
        playSound(const std::string& path, const std::string& asset_folder, bool loop = false, float volume = 1.0f);

        ma_engine* getEngine() const { return m_engine.get(); }

        void clearSceneSounds();

    private:
        void startSoundForEntity(Scene* scene, entt::entity entity, const std::string& asset_folder);
        void stopSoundForEntity(entt::entity entity);
        void updateSoundPosition(Scene* scene, entt::entity entity);

        std::unique_ptr<ma_engine, void (*)(ma_engine*)> m_engine;
        std::unordered_map<entt::entity, ma_sound*>      m_entity_sounds;
    };

} // namespace RealmEngine
