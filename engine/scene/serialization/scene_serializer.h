#pragma once

#include <entt/entity/entity.hpp>
#include <json.hpp>
#include <memory>
#include <string>
#include <unordered_set>

namespace RealmEngine
{
    class AssetManager;
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

        static std::string serialize(const std::shared_ptr<Scene>& scene);
        static std::shared_ptr<Scene>
        deserialize(const std::string& json, RHIDevice& device, AssetManager* asset_mgr = nullptr);

        static bool saveToFile(std::shared_ptr<Scene> scene, const std::string& filepath, bool encrypt = false);
        static std::shared_ptr<Scene> loadFromFile(const std::string& filepath,
                                                   RHIDevice&         device,
                                                   AssetManager*      asset_mgr = nullptr,
                                                   bool               encrypted = false);

        static std::string                serializeNodeToJson(const std::shared_ptr<SceneNode>& node, Scene& scene);
        static std::shared_ptr<SceneNode> pasteNodeFromJson(const std::string&                json,
                                                            Scene&                            scene,
                                                            const std::shared_ptr<SceneNode>& parent,
                                                            RHIDevice&                        device,
                                                            AssetManager*                     asset_mgr);

    private:
        static void serializeNode(nlohmann::json& json, const std::shared_ptr<SceneNode>& node, Scene& scene);
        static void serializeEntity(nlohmann::json& json, entt::entity entity, Scene& scene);

        static std::shared_ptr<SceneNode>
        deserializeNode(const nlohmann::json& json, Scene& scene, RHIDevice& device, AssetManager* asset_mgr);
        static std::shared_ptr<SceneNode> deserializeNodeWithUniquify(const nlohmann::json&            node_json,
                                                                      Scene&                           scene,
                                                                      RHIDevice&                       device,
                                                                      AssetManager*                    asset_mgr,
                                                                      std::unordered_set<std::string>& used);
        static void                       deserializeEntity(const nlohmann::json& json,
                                                            Scene&                scene,
                                                            const std::string&    name,
                                                            RHIDevice&            device,
                                                            AssetManager*         asset_mgr);
    };

} // namespace RealmEngine
