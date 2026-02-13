#pragma once

#include <json.hpp>
#include <memory>
#include <string>
#include "scene/scene.h"

namespace RealmEngine
{
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
        static std::shared_ptr<Scene> deserialize(const std::string& json);

        static bool saveToFile(std::shared_ptr<Scene> scene, const std::string& filepath, bool encrypt = false);
        static std::shared_ptr<Scene> loadFromFile(const std::string& filepath, bool encrypted = false);

    private:
        static void serializeNode(nlohmann::json& json, std::shared_ptr<SceneNode> node, std::shared_ptr<Scene> scene);
        static void serializeEntity(nlohmann::json& json, std::shared_ptr<Entity> entity);
        static void serializeComponent(nlohmann::json& json, std::shared_ptr<Component> component);

        static std::shared_ptr<SceneNode> deserializeNode(const nlohmann::json& json, std::shared_ptr<Scene> scene);
        static void
        deserializeEntity(const nlohmann::json& json, std::shared_ptr<Scene> scene, const std::string& name);
        static std::shared_ptr<Component> deserializeComponent(const nlohmann::json& json);
    };

} // namespace RealmEngine
