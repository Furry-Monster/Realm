#include "scene/serialization/scene_serializer.h"

#include <fstream>
#include <json.hpp>
#include <sstream>

#include "core/base/utils.h"
#include "core/log/log_macros.h"
#include "resource/asset_manager.h"
#include "scene/components/lighting/area.h"
#include "scene/components/lighting/directional.h"
#include "scene/components/lighting/point.h"
#include "scene/components/lighting/spot.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/scene.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    std::string SceneSerializer::serialize(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return "{}";

        nlohmann::json json;
        json["version"] = 1;

        nlohmann::json root_json;
        serializeNode(root_json, scene->getRoot(), *scene);
        json["root"] = root_json;

        return json.dump(2);
    }

    std::shared_ptr<Scene>
    SceneSerializer::deserialize(const std::string& json, RHIDevice& device, AssetManager* asset_mgr)
    {
        if (json.empty())
            return nullptr;

        try
        {
            nlohmann::json json_obj = nlohmann::json::parse(json);

            auto scene = std::make_shared<Scene>();

            if (json_obj.contains("root"))
            {
                auto root = deserializeNode(json_obj["root"], *scene, device, asset_mgr);
                if (root)
                {
                    scene->getRoot()->clearChildren();
                    std::vector<std::shared_ptr<SceneNode>> children;
                    root->forEachChild([&children](std::shared_ptr<SceneNode> child) { children.push_back(child); });
                    for (auto& child : children)
                        scene->getRoot()->addChild(child);
                }
            }

            return scene;
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to deserialize scene: " + std::string(e.what()));
            return nullptr;
        }
    }

    bool SceneSerializer::saveToFile(std::shared_ptr<Scene> scene, const std::string& filepath, bool encrypt)
    {
        try
        {
            std::string json_str = serialize(scene);
            std::string output   = json_str;

            if (encrypt)
            {
                std::string encrypted = xorEncrypt(json_str, DEFAULT_ENCRYPTION_KEY);
                output                = base64Encode(encrypted);
            }

            std::ofstream file(filepath);
            if (!file.is_open())
                return false;

            file << output;
            file.close();
            return true;
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to save scene file: " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<Scene> SceneSerializer::loadFromFile(const std::string& filepath,
                                                         RHIDevice&         device,
                                                         AssetManager*      asset_mgr,
                                                         bool               encrypted)
    {
        try
        {
            std::ifstream file(filepath);
            if (!file.is_open())
                return nullptr;

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

            return deserialize(json_str, device, asset_mgr);
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to load scene file: " + std::string(e.what()));
            return nullptr;
        }
    }

    void SceneSerializer::serializeNode(nlohmann::json& json, std::shared_ptr<SceneNode> node, Scene& scene)
    {
        if (!node)
            return;

        json["name"] = node->getName();

        if (node->hasEntity())
        {
            entt::entity entity = node->getEntity();
            if (scene.valid(entity))
            {
                nlohmann::json entity_json;
                serializeEntity(entity_json, entity, scene);
                json["entity"] = entity_json;
            }
        }

        if (node->getChildCount() > 0)
        {
            nlohmann::json children_json = nlohmann::json::array();
            node->forEachChild([&children_json, &scene](const std::shared_ptr<SceneNode>& child) {
                nlohmann::json child_json;
                serializeNode(child_json, child, scene);
                children_json.push_back(child_json);
            });
            json["children"] = children_json;
        }
    }

    void SceneSerializer::serializeEntity(nlohmann::json& json, entt::entity entity, Scene& scene)
    {
        json["id"] = static_cast<uint32_t>(entity);

        nlohmann::json components_json = nlohmann::json::array();

        if (auto* tf = scene.tryGet<Transform>(entity))
        {
            nlohmann::json c;
            c["type"]     = "Transform";
            auto euler    = tf->getEulerAngles();
            c["position"] = nlohmann::json::array({tf->position.x, tf->position.y, tf->position.z});
            c["rotation"] = nlohmann::json::array({euler.x, euler.y, euler.z});
            c["scale"]    = nlohmann::json::array({tf->scale.x, tf->scale.y, tf->scale.z});
            components_json.push_back(c);
        }

        if (auto* r = scene.tryGet<Renderable>(entity))
        {
            nlohmann::json c;
            c["type"]          = "Renderable";
            c["model_path"]    = r->model_path.empty() ? "" : r->model_path;
            c["flip_textures"] = r->flip_textures;
            components_json.push_back(c);
        }

        if (auto* pl = scene.tryGet<PointLight>(entity))
        {
            nlohmann::json c;
            c["type"]      = "Point";
            c["color"]     = nlohmann::json::array({pl->color.x, pl->color.y, pl->color.z});
            c["intensity"] = pl->intensity;
            c["enabled"]   = pl->enabled;
            c["range"]     = pl->range;
            c["constant"]  = pl->constant;
            c["linear"]    = pl->linear;
            c["quadratic"] = pl->quadratic;
            components_json.push_back(c);
        }

        if (auto* sl = scene.tryGet<SpotLight>(entity))
        {
            nlohmann::json c;
            c["type"]             = "Spot";
            c["color"]            = nlohmann::json::array({sl->color.x, sl->color.y, sl->color.z});
            c["intensity"]        = sl->intensity;
            c["enabled"]          = sl->enabled;
            c["range"]            = sl->range;
            c["constant"]         = sl->constant;
            c["linear"]           = sl->linear;
            c["quadratic"]        = sl->quadratic;
            c["inner_cone_angle"] = sl->inner_cone_angle;
            c["outer_cone_angle"] = sl->outer_cone_angle;
            components_json.push_back(c);
        }

        if (auto* dl = scene.tryGet<DirectionalLight>(entity))
        {
            nlohmann::json c;
            c["type"]      = "Directional";
            c["color"]     = nlohmann::json::array({dl->color.x, dl->color.y, dl->color.z});
            c["intensity"] = dl->intensity;
            c["enabled"]   = dl->enabled;
            components_json.push_back(c);
        }

        if (auto* al = scene.tryGet<AreaLight>(entity))
        {
            nlohmann::json c;
            c["type"]      = "Area";
            c["color"]     = nlohmann::json::array({al->color.x, al->color.y, al->color.z});
            c["intensity"] = al->intensity;
            c["enabled"]   = al->enabled;
            c["width"]     = al->width;
            c["height"]    = al->height;
            components_json.push_back(c);
        }

        json["components"] = components_json;
    }

    std::shared_ptr<SceneNode> SceneSerializer::deserializeNode(const nlohmann::json& json,
                                                                Scene&                scene,
                                                                RHIDevice&            device,
                                                                AssetManager*         asset_mgr)
    {
        if (!json.contains("name"))
            return nullptr;

        std::string name = json["name"];
        auto        node = std::make_shared<SceneNode>(name);

        if (json.contains("entity"))
        {
            deserializeEntity(json["entity"], scene, name, device, asset_mgr);
            auto entity = scene.findEntity(name);
            if (entity)
                node->setEntity(entity.handle());
        }

        if (json.contains("children") && json["children"].is_array())
        {
            for (const auto& child_json : json["children"])
            {
                auto child = deserializeNode(child_json, scene, device, asset_mgr);
                if (child)
                    node->addChild(child);
            }
        }

        return node;
    }

    void SceneSerializer::deserializeEntity(const nlohmann::json& json,
                                            Scene&                scene,
                                            const std::string&    name,
                                            RHIDevice&            device,
                                            AssetManager*         asset_mgr)
    {
        if (!json.contains("id") || !json.contains("components"))
            return;

        auto entity = scene.createEntity(name);

        for (const auto& comp_json : json["components"])
        {
            if (!comp_json.contains("type"))
                continue;

            std::string type = comp_json["type"];

            if (type == "Transform")
            {
                auto& tf = entity.emplace<Transform>();

                if (comp_json.contains("position") && comp_json["position"].is_array() &&
                    comp_json["position"].size() == 3)
                    tf.position =
                        glm::vec3(comp_json["position"][0], comp_json["position"][1], comp_json["position"][2]);

                if (comp_json.contains("rotation") && comp_json["rotation"].is_array() &&
                    comp_json["rotation"].size() == 3)
                    tf.rotation = glm::quat(
                        glm::vec3(comp_json["rotation"][0], comp_json["rotation"][1], comp_json["rotation"][2]));

                if (comp_json.contains("scale") && comp_json["scale"].is_array() && comp_json["scale"].size() == 3)
                    tf.scale = glm::vec3(comp_json["scale"][0], comp_json["scale"][1], comp_json["scale"][2]);
            }
            else if (type == "Renderable")
            {
                auto& r = entity.emplace<Renderable>();

                if (comp_json.contains("model_path") && comp_json["model_path"].is_string())
                    r.model_path = comp_json["model_path"];

                if (comp_json.contains("flip_textures") && comp_json["flip_textures"].is_boolean())
                    r.flip_textures = comp_json["flip_textures"];

                if (asset_mgr)
                    r.loadModel(device, *asset_mgr);
                else
                    r.loadModel(device);
            }
            else if (type == "Point")
            {
                auto& pl = entity.emplace<PointLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    pl.color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    pl.intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    pl.enabled = comp_json["enabled"];
                if (comp_json.contains("range") && comp_json["range"].is_number())
                    pl.range = comp_json["range"];
                if (comp_json.contains("constant") && comp_json["constant"].is_number())
                    pl.constant = comp_json["constant"];
                if (comp_json.contains("linear") && comp_json["linear"].is_number())
                    pl.linear = comp_json["linear"];
                if (comp_json.contains("quadratic") && comp_json["quadratic"].is_number())
                    pl.quadratic = comp_json["quadratic"];
            }
            else if (type == "Spot")
            {
                auto& sl = entity.emplace<SpotLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    sl.color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    sl.intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    sl.enabled = comp_json["enabled"];
                if (comp_json.contains("range") && comp_json["range"].is_number())
                    sl.range = comp_json["range"];
                if (comp_json.contains("constant") && comp_json["constant"].is_number())
                    sl.constant = comp_json["constant"];
                if (comp_json.contains("linear") && comp_json["linear"].is_number())
                    sl.linear = comp_json["linear"];
                if (comp_json.contains("quadratic") && comp_json["quadratic"].is_number())
                    sl.quadratic = comp_json["quadratic"];
                if (comp_json.contains("inner_cone_angle") && comp_json["inner_cone_angle"].is_number())
                    sl.inner_cone_angle = comp_json["inner_cone_angle"];
                if (comp_json.contains("outer_cone_angle") && comp_json["outer_cone_angle"].is_number())
                    sl.outer_cone_angle = comp_json["outer_cone_angle"];
            }
            else if (type == "Directional")
            {
                auto& dl = entity.emplace<DirectionalLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    dl.color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    dl.intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    dl.enabled = comp_json["enabled"];
            }
            else if (type == "Area")
            {
                auto& al = entity.emplace<AreaLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    al.color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    al.intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    al.enabled = comp_json["enabled"];
                if (comp_json.contains("width") && comp_json["width"].is_number())
                    al.width = comp_json["width"];
                if (comp_json.contains("height") && comp_json["height"].is_number())
                    al.height = comp_json["height"];
            }
        }
    }

} // namespace RealmEngine
