#include "gameplay/scene/scene_serializer.h"

#include <fstream>
#include <iostream>
#include <json.hpp>
#include <sstream>

#include "gameplay/components/lighting/area.h"
#include "gameplay/components/lighting/directional.h"
#include "gameplay/components/lighting/point.h"
#include "gameplay/components/lighting/spot.h"
#include "gameplay/components/renderable.h"
#include "gameplay/components/transform.h"
#include "gameplay/entity.h"
#include "gameplay/scene/scene_node.h"
#include "utils.h"

namespace RealmEngine
{
    std::string SceneSerializer::serialize(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return "{}";

        nlohmann::json json;
        json["version"] = 1;

        nlohmann::json root_json;
        serializeNode(root_json, scene->getRoot(), scene);
        json["root"] = root_json;

        return json.dump(2);
    }

    std::shared_ptr<Scene> SceneSerializer::deserialize(const std::string& json)
    {
        if (json.empty())
            return nullptr;

        try
        {
            nlohmann::json json_obj = nlohmann::json::parse(json);

            auto scene = std::make_shared<Scene>();

            if (json_obj.contains("root"))
            {
                auto root = deserializeNode(json_obj["root"], scene);
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
            err("Failed to deserialize scene: " + std::string(e.what()));
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
            err("Failed to save scene file: " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<Scene> SceneSerializer::loadFromFile(const std::string& filepath, bool encrypted)
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

            return deserialize(json_str);
        }
        catch (const std::exception& e)
        {
            err("Failed to load scene file: " + std::string(e.what()));
            return nullptr;
        }
    }

    void
    SceneSerializer::serializeNode(nlohmann::json& json, std::shared_ptr<SceneNode> node, std::shared_ptr<Scene> scene)
    {
        if (!node)
            return;

        json["name"] = node->getName();

        if (node->hasEntity())
        {
            size_t entity_id = node->getEntityId();
            auto   entity    = scene->getEntity(entity_id);
            if (entity)
            {
                nlohmann::json entity_json;
                serializeEntity(entity_json, entity);
                json["entity"] = entity_json;
            }
        }

        if (node->getChildCount() > 0)
        {
            nlohmann::json children_json = nlohmann::json::array();
            node->forEachChild([&children_json, scene](const std::shared_ptr<SceneNode>& child) {
                nlohmann::json child_json;
                serializeNode(child_json, child, scene);
                children_json.push_back(child_json);
            });
            json["children"] = children_json;
        }
    }

    void SceneSerializer::serializeEntity(nlohmann::json& json, std::shared_ptr<Entity> entity)
    {
        if (!entity)
            return;

        json["id"] = entity->getId();

        nlohmann::json components_json = nlohmann::json::array();

        auto transform = entity->getComponent<Transform>();
        if (transform)
        {
            nlohmann::json comp_json;
            serializeComponent(comp_json, transform);
            components_json.push_back(comp_json);
        }

        auto renderable = entity->getComponent<Renderable>();
        if (renderable)
        {
            nlohmann::json comp_json;
            serializeComponent(comp_json, renderable);
            components_json.push_back(comp_json);
        }

        auto point_light = entity->getComponent<Point>();
        if (point_light)
        {
            nlohmann::json comp_json;
            serializeComponent(comp_json, point_light);
            components_json.push_back(comp_json);
        }

        auto spot_light = entity->getComponent<Spot>();
        if (spot_light)
        {
            nlohmann::json comp_json;
            serializeComponent(comp_json, spot_light);
            components_json.push_back(comp_json);
        }

        auto directional_light = entity->getComponent<Directional>();
        if (directional_light)
        {
            nlohmann::json comp_json;
            serializeComponent(comp_json, directional_light);
            components_json.push_back(comp_json);
        }

        auto area_light = entity->getComponent<Area>();
        if (area_light)
        {
            nlohmann::json comp_json;
            serializeComponent(comp_json, area_light);
            components_json.push_back(comp_json);
        }

        json["components"] = components_json;
    }

    void SceneSerializer::serializeComponent(nlohmann::json& json, std::shared_ptr<Component> component)
    {
        if (!component)
            return;

        auto transform = std::dynamic_pointer_cast<Transform>(component);
        if (transform)
        {
            json["type"] = "Transform";
            auto pos     = transform->getPosition();
            auto rot     = transform->getEulerAngles();
            auto scale   = transform->getScale();

            json["position"] = nlohmann::json::array({pos.x, pos.y, pos.z});
            json["rotation"] = nlohmann::json::array({rot.x, rot.y, rot.z});
            json["scale"]    = nlohmann::json::array({scale.x, scale.y, scale.z});
            return;
        }

        auto renderable = std::dynamic_pointer_cast<Renderable>(component);
        if (renderable)
        {
            json["type"] = "Renderable";
            if (renderable->hasModelPath())
                json["model_path"] = renderable->getModelPath();
            else
                json["model_path"] = "";
            json["flip_textures"] = false;
            return;
        }

        auto point_light = std::dynamic_pointer_cast<Point>(component);
        if (point_light)
        {
            json["type"]      = "Point";
            auto color        = point_light->getColor();
            json["color"]     = nlohmann::json::array({color.x, color.y, color.z});
            json["intensity"] = point_light->getIntensity();
            json["enabled"]   = point_light->isEnabled();
            json["range"]     = point_light->getRange();
            json["constant"]  = point_light->getConstantAttenuation();
            json["linear"]    = point_light->getLinearAttenuation();
            json["quadratic"] = point_light->getQuadraticAttenuation();
            return;
        }

        auto spot_light = std::dynamic_pointer_cast<Spot>(component);
        if (spot_light)
        {
            json["type"]             = "Spot";
            auto color               = spot_light->getColor();
            json["color"]            = nlohmann::json::array({color.x, color.y, color.z});
            json["intensity"]        = spot_light->getIntensity();
            json["enabled"]          = spot_light->isEnabled();
            json["range"]            = spot_light->getRange();
            json["constant"]         = spot_light->getConstantAttenuation();
            json["linear"]           = spot_light->getLinearAttenuation();
            json["quadratic"]        = spot_light->getQuadraticAttenuation();
            json["inner_cone_angle"] = spot_light->getInnerConeAngle();
            json["outer_cone_angle"] = spot_light->getOuterConeAngle();
            return;
        }

        auto directional_light = std::dynamic_pointer_cast<Directional>(component);
        if (directional_light)
        {
            json["type"]      = "Directional";
            auto color        = directional_light->getColor();
            json["color"]     = nlohmann::json::array({color.x, color.y, color.z});
            json["intensity"] = directional_light->getIntensity();
            json["enabled"]   = directional_light->isEnabled();
            return;
        }

        auto area_light = std::dynamic_pointer_cast<Area>(component);
        if (area_light)
        {
            json["type"]      = "Area";
            auto color        = area_light->getColor();
            json["color"]     = nlohmann::json::array({color.x, color.y, color.z});
            json["intensity"] = area_light->getIntensity();
            json["enabled"]   = area_light->isEnabled();
            json["width"]     = area_light->getWidth();
            json["height"]    = area_light->getHeight();
            return;
        }
    }

    std::shared_ptr<SceneNode> SceneSerializer::deserializeNode(const nlohmann::json&  json,
                                                                std::shared_ptr<Scene> scene)
    {
        if (!json.contains("name"))
            return nullptr;

        std::string name = json["name"];
        auto        node = std::make_shared<SceneNode>(name);

        if (json.contains("entity"))
        {
            deserializeEntity(json["entity"], scene, name);
            if (scene->hasEntity(name))
            {
                size_t entity_id = Scene::hashName(name);
                node->setEntityId(entity_id);
            }
        }

        if (json.contains("children") && json["children"].is_array())
        {
            for (const auto& child_json : json["children"])
            {
                auto child = deserializeNode(child_json, scene);
                if (child)
                    node->addChild(child);
            }
        }

        return node;
    }

    void SceneSerializer::deserializeEntity(const nlohmann::json&  json,
                                            std::shared_ptr<Scene> scene,
                                            const std::string&     name)
    {
        if (!json.contains("id") || !json.contains("components"))
            return;

        auto entity = scene->createEntity(name);

        for (const auto& comp_json : json["components"])
        {
            auto component = deserializeComponent(comp_json);
            if (component)
                entity->addComponent(component);
        }
    }

    std::shared_ptr<Component> SceneSerializer::deserializeComponent(const nlohmann::json& json)
    {
        if (!json.contains("type"))
            return nullptr;

        std::string type = json["type"];

        if (type == "Transform")
        {
            auto transform = std::make_shared<Transform>();

            if (json.contains("position") && json["position"].is_array() && json["position"].size() == 3)
                transform->setPosition(glm::vec3(json["position"][0], json["position"][1], json["position"][2]));

            if (json.contains("rotation") && json["rotation"].is_array() && json["rotation"].size() == 3)
                transform->setRotation(glm::vec3(json["rotation"][0], json["rotation"][1], json["rotation"][2]));

            if (json.contains("scale") && json["scale"].is_array() && json["scale"].size() == 3)
                transform->setScale(glm::vec3(json["scale"][0], json["scale"][1], json["scale"][2]));

            return transform;
        }

        if (type == "Point")
        {
            auto point_light = std::make_shared<Point>();

            if (json.contains("color") && json["color"].is_array() && json["color"].size() == 3)
            {
                glm::vec3 color = glm::vec3(json["color"][0], json["color"][1], json["color"][2]);
                point_light->setColor(color);
            }

            if (json.contains("intensity") && json["intensity"].is_number())
                point_light->setIntensity(json["intensity"]);

            if (json.contains("enabled") && json["enabled"].is_boolean())
                point_light->setEnabled(json["enabled"]);

            if (json.contains("range") && json["range"].is_number())
                point_light->setRange(json["range"]);

            if (json.contains("constant") && json["constant"].is_number())
                point_light->setConstantAttenuation(json["constant"]);

            if (json.contains("linear") && json["linear"].is_number())
                point_light->setLinearAttenuation(json["linear"]);

            if (json.contains("quadratic") && json["quadratic"].is_number())
                point_light->setQuadraticAttenuation(json["quadratic"]);

            return point_light;
        }

        if (type == "Spot")
        {
            auto spot_light = std::make_shared<Spot>();

            if (json.contains("color") && json["color"].is_array() && json["color"].size() == 3)
            {
                glm::vec3 color = glm::vec3(json["color"][0], json["color"][1], json["color"][2]);
                spot_light->setColor(color);
            }

            if (json.contains("intensity") && json["intensity"].is_number())
                spot_light->setIntensity(json["intensity"]);

            if (json.contains("enabled") && json["enabled"].is_boolean())
                spot_light->setEnabled(json["enabled"]);

            if (json.contains("range") && json["range"].is_number())
                spot_light->setRange(json["range"]);

            if (json.contains("constant") && json["constant"].is_number())
                spot_light->setConstantAttenuation(json["constant"]);

            if (json.contains("linear") && json["linear"].is_number())
                spot_light->setLinearAttenuation(json["linear"]);

            if (json.contains("quadratic") && json["quadratic"].is_number())
                spot_light->setQuadraticAttenuation(json["quadratic"]);

            if (json.contains("inner_cone_angle") && json["inner_cone_angle"].is_number())
                spot_light->setInnerConeAngle(json["inner_cone_angle"]);

            if (json.contains("outer_cone_angle") && json["outer_cone_angle"].is_number())
                spot_light->setOuterConeAngle(json["outer_cone_angle"]);

            return spot_light;
        }

        if (type == "Directional")
        {
            auto directional_light = std::make_shared<Directional>();

            if (json.contains("color") && json["color"].is_array() && json["color"].size() == 3)
            {
                glm::vec3 color = glm::vec3(json["color"][0], json["color"][1], json["color"][2]);
                directional_light->setColor(color);
            }

            if (json.contains("intensity") && json["intensity"].is_number())
                directional_light->setIntensity(json["intensity"]);

            if (json.contains("enabled") && json["enabled"].is_boolean())
                directional_light->setEnabled(json["enabled"]);

            return directional_light;
        }

        if (type == "Area")
        {
            auto area_light = std::make_shared<Area>();

            if (json.contains("color") && json["color"].is_array() && json["color"].size() == 3)
            {
                glm::vec3 color = glm::vec3(json["color"][0], json["color"][1], json["color"][2]);
                area_light->setColor(color);
            }

            if (json.contains("intensity") && json["intensity"].is_number())
                area_light->setIntensity(json["intensity"]);

            if (json.contains("enabled") && json["enabled"].is_boolean())
                area_light->setEnabled(json["enabled"]);

            if (json.contains("width") && json["width"].is_number())
                area_light->setWidth(json["width"]);

            if (json.contains("height") && json["height"].is_number())
                area_light->setHeight(json["height"]);

            return area_light;
        }

        if (type == "Renderable")
        {
            std::string model_path;
            bool        flip_textures = false;

            if (json.contains("model_path") && json["model_path"].is_string())
                model_path = json["model_path"];

            if (json.contains("flip_textures") && json["flip_textures"].is_boolean())
                flip_textures = json["flip_textures"];

            if (!model_path.empty())
                return std::make_shared<Renderable>(model_path, flip_textures);
            else
                return std::make_shared<Renderable>();
        }

        return nullptr;
    }

} // namespace RealmEngine
