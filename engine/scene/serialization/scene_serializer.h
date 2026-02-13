#pragma once

#include <entt/entity/entity.hpp>
#include <json.hpp>
#include <memory>
#include <string>

namespace RealmEngine
{
    class Scene;
    class SceneNode;
    class RHIDevice;

    class SceneSerializer
    {
    public:
        SceneSerializer()           = default;
        ~SceneSerializer() noexcept = default;

        SceneSerializer(const SceneSerializer&)                = delete;
        SceneSerializer& operator=(const SceneSerializer&)     = delete;
        SceneSerializer(SceneSerializer&&) noexcept            = default;
        SceneSerializer& operator=(SceneSerializer&&) noexcept = default;

        static std::string            serialize(std::shared_ptr<Scene> scene);
        static std::shared_ptr<Scene> deserialize(const std::string& json, RHIDevice& device);

        static bool saveToFile(std::shared_ptr<Scene> scene, const std::string& filepath, bool encrypt = false);
        static std::shared_ptr<Scene>
        loadFromFile(const std::string& filepath, RHIDevice& device, bool encrypted = false);

    private:
        static void serializeNode(nlohmann::json& json, std::shared_ptr<SceneNode> node, Scene& scene);
        static void serializeEntity(nlohmann::json& json, entt::entity entity, Scene& scene);

        static std::shared_ptr<SceneNode> deserializeNode(const nlohmann::json& json, Scene& scene, RHIDevice& device);
        static void
        deserializeEntity(const nlohmann::json& json, Scene& scene, const std::string& name, RHIDevice& device);
    };

} // namespace RealmEngine
