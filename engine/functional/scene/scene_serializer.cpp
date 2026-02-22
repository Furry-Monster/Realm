#include "functional/scene/scene_serializer.h"

#include <fstream>
#include <json.hpp>
#include <sstream>
#include <unordered_set>

#include "core/base/macros.h"
#include "core/base/utils.h"
#include "module/ecs/components/audio/audio_listener.h"
#include "module/ecs/components/audio/audio_source.h"
#include "module/ecs/components/camera.h"
#include "module/ecs/components/lighting/area.h"
#include "module/ecs/components/lighting/directional.h"
#include "module/ecs/components/lighting/point.h"
#include "module/ecs/components/lighting/spot.h"
#include "module/ecs/components/renderable.h"
#include "module/ecs/components/transform.h"
#include "module/render/material.h"
#include "module/render/render_mesh.h"
#include "module/render/render_object.h"
#include "functional/resource/asset_manager.h"
#include "functional/scene/scene.h"
#include "functional/scene/scene_node.h"

namespace RealmEngine
{
    static constexpr int SCENE_FORMAT_VERSION = 1;

    std::string SceneSerializer::serialize(const std::shared_ptr<Scene>& scene)
    {
        if (!scene)
            return "{}";

        nlohmann::json json;
        json["version"] = SCENE_FORMAT_VERSION;

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

            int version = 0;
            if (json_obj.contains("version") && json_obj["version"].is_number_integer())
                version = json_obj["version"].get<int>();

            if (version > SCENE_FORMAT_VERSION)
            {
                RE_LOG_ERROR("Scene file version " + std::to_string(version) + " is newer than supported version " +
                             std::to_string(SCENE_FORMAT_VERSION) + ". Please update RealmEngine.");
                return nullptr;
            }
            if (version < 1)
            {
                RE_LOG_WARN("Scene file has no version field; assuming version 1.");
            }

            auto scene = std::make_shared<Scene>();

            if (json_obj.contains("root"))
            {
                const auto root = deserializeNode(json_obj["root"], *scene, device, asset_mgr);
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

    void SceneSerializer::serializeNode(nlohmann::json& json, const std::shared_ptr<SceneNode>& node, Scene& scene)
    {
        if (!node)
            return;

        json["name"] = node->getName();

        if (node->hasEntity())
        {
            const entt::entity entity = node->getEntity();
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
            c["position"] = nlohmann::json::array({tf->position.x, tf->position.y, tf->position.z});
            c["rotation"] = nlohmann::json::array({tf->rotation.w, tf->rotation.x, tf->rotation.y, tf->rotation.z});
            c["scale"]    = nlohmann::json::array({tf->scale.x, tf->scale.y, tf->scale.z});
            components_json.push_back(c);
        }

        if (auto* r = scene.tryGet<Renderable>(entity))
        {
            nlohmann::json c;
            c["type"]          = "Renderable";
            c["model_path"]    = r->model_path.empty() ? "" : r->model_path;
            c["flip_textures"] = r->flip_textures;
            if (r->render_object)
            {
                nlohmann::json overrides = nlohmann::json::array();
                for (size_t i = 0; i < r->render_object->getMeshCount(); ++i)
                {
                    auto* mesh = r->render_object->getMesh(i);
                    if (!mesh)
                        continue;
                    const auto&    mat   = mesh->m_material;
                    const auto&    props = mat.properties;
                    nlohmann::json m;

                    m["shading_model"] = static_cast<int>(mat.shading_model);
                    m["blend_mode"]    = static_cast<int>(mat.blend_mode);
                    m["render_face"]   = static_cast<int>(mat.render_face);
                    m["alpha_cutoff"]  = mat.alpha_cutoff;
                    m["render_queue"]  = mat.render_queue;
                    m["vert_path"]     = mat.vert_path;
                    m["frag_path"]     = mat.frag_path;

                    nlohmann::json props_json = nlohmann::json::array();
                    for (const auto& [name, prop] : props.getProperties())
                    {
                        if (prop.type == PropType::Texture2D)
                            continue;
                        nlohmann::json pj;
                        pj["name"]   = name;
                        pj["type"]   = static_cast<int>(prop.type);
                        pj["values"] = {prop.values[0], prop.values[1], prop.values[2], prop.values[3]};
                        props_json.push_back(pj);
                    }
                    m["properties"] = props_json;

                    overrides.push_back(m);
                }
                c["mesh_overrides"] = overrides;
            }
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

        if (auto* cam = scene.tryGet<Camera>(entity))
        {
            nlohmann::json c;
            c["type"]      = "Camera";
            c["fov"]       = cam->fov;
            c["near"]      = cam->near_plane;
            c["far"]       = cam->far_plane;
            c["primary"]   = cam->primary;
            c["proj_type"] = static_cast<int>(cam->projection_type);
            if (cam->projection_type == CameraProjectionType::Orthographic)
            {
                c["ortho_left"]   = cam->ortho_left;
                c["ortho_right"]  = cam->ortho_right;
                c["ortho_bottom"] = cam->ortho_bottom;
                c["ortho_top"]    = cam->ortho_top;
            }
            components_json.push_back(c);
        }

        if (auto* as = scene.tryGet<AudioSource>(entity))
        {
            nlohmann::json c;
            c["type"]          = "AudioSource";
            c["clip_path"]     = as->clip_path;
            c["volume"]        = as->volume;
            c["loop"]          = as->loop;
            c["spatial"]       = as->spatial;
            c["play_on_start"] = as->play_on_start;
            components_json.push_back(c);
        }

        if (auto* al = scene.tryGet<AudioListener>(entity))
        {
            nlohmann::json c;
            c["type"]    = "AudioListener";
            c["primary"] = al->primary;
            components_json.push_back(c);
        }

        json["components"] = components_json;
    }

    std::string SceneSerializer::serializeNodeToJson(const std::shared_ptr<SceneNode>& node, Scene& scene)
    {
        if (!node)
            return "{}";
        nlohmann::json j;
        serializeNode(j, node, scene);
        return j.dump(2);
    }

    namespace
    {
        std::string findUniqueName(const Scene& scene, std::unordered_set<std::string>& used, const std::string& base)
        {
            std::string name = base;
            int         n    = 0;
            while (scene.findEntity(name) || used.count(name))
            {
                name = base + " (" + std::to_string(++n) + ")";
            }
            used.insert(name);
            return name;
        }
    } // namespace

    std::shared_ptr<SceneNode> SceneSerializer::deserializeNodeWithUniquify(const nlohmann::json&            node_json,
                                                                            Scene&                           scene,
                                                                            RHIDevice&                       device,
                                                                            AssetManager*                    asset_mgr,
                                                                            std::unordered_set<std::string>& used)
    {
        if (!node_json.contains("name"))
            return nullptr;
        const std::string base_name = node_json["name"];
        std::string       name      = findUniqueName(scene, used, base_name);
        auto              node      = std::make_shared<SceneNode>(name);

        if (node_json.contains("entity"))
        {
            deserializeEntity(node_json["entity"], scene, name, device, asset_mgr);
            auto entity = scene.findEntity(name);
            if (entity)
                node->setEntity(entity.handle());
        }

        if (node_json.contains("children") && node_json["children"].is_array())
        {
            for (const auto& child_json : node_json["children"])
            {
                auto child = deserializeNodeWithUniquify(child_json, scene, device, asset_mgr, used);
                if (child)
                    node->addChild(child);
            }
        }
        return node;
    }

    std::shared_ptr<SceneNode> SceneSerializer::pasteNodeFromJson(const std::string&                json,
                                                                  Scene&                            scene,
                                                                  const std::shared_ptr<SceneNode>& parent,
                                                                  RHIDevice&                        device,
                                                                  AssetManager*                     asset_mgr)
    {
        if (json.empty() || !parent)
            return nullptr;
        try
        {
            nlohmann::json                  j = nlohmann::json::parse(json);
            std::unordered_set<std::string> used;
            auto                            node = deserializeNodeWithUniquify(j, scene, device, asset_mgr, used);
            if (node)
            {
                parent->addChild(node);
                return node;
            }
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to paste entity: " + std::string(e.what()));
        }
        return nullptr;
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
            const auto entity = scene.findEntity(name);
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
                auto& [position, rotation, scale] = entity.emplace<Transform>();

                if (comp_json.contains("position") && comp_json["position"].is_array() &&
                    comp_json["position"].size() == 3)
                    position = glm::vec3(comp_json["position"][0], comp_json["position"][1], comp_json["position"][2]);

                if (comp_json.contains("rotation") && comp_json["rotation"].is_array())
                {
                    if (comp_json["rotation"].size() == 4)
                    {
                        rotation = glm::quat(comp_json["rotation"][0],
                                             comp_json["rotation"][1],
                                             comp_json["rotation"][2],
                                             comp_json["rotation"][3]);
                    }
                    else if (comp_json["rotation"].size() == 3)
                    {
                        rotation = glm::quat(
                            glm::vec3(comp_json["rotation"][0], comp_json["rotation"][1], comp_json["rotation"][2]));
                    }
                }

                if (comp_json.contains("scale") && comp_json["scale"].is_array() && comp_json["scale"].size() == 3)
                    scale = glm::vec3(comp_json["scale"][0], comp_json["scale"][1], comp_json["scale"][2]);
            }
            else if (type == "Camera")
            {
                auto& c = entity.emplace<Camera>();
                if (comp_json.contains("fov") && comp_json["fov"].is_number())
                    c.fov = comp_json["fov"];
                if (comp_json.contains("near") && comp_json["near"].is_number())
                    c.near_plane = comp_json["near"];
                if (comp_json.contains("far") && comp_json["far"].is_number())
                    c.far_plane = comp_json["far"];
                if (comp_json.contains("primary") && comp_json["primary"].is_boolean())
                    c.primary = comp_json["primary"];
                if (comp_json.contains("proj_type") && comp_json["proj_type"].is_number_integer())
                    c.projection_type = static_cast<CameraProjectionType>(comp_json["proj_type"].get<int>());
                if (c.projection_type == CameraProjectionType::Orthographic)
                {
                    if (comp_json.contains("ortho_left"))
                        c.ortho_left = comp_json["ortho_left"];
                    if (comp_json.contains("ortho_right"))
                        c.ortho_right = comp_json["ortho_right"];
                    if (comp_json.contains("ortho_bottom"))
                        c.ortho_bottom = comp_json["ortho_bottom"];
                    if (comp_json.contains("ortho_top"))
                        c.ortho_top = comp_json["ortho_top"];
                }
            }
            else if (type == "Renderable")
            {
                auto& r = entity.emplace<Renderable>();

                if (comp_json.contains("model_path") && comp_json["model_path"].is_string())
                    r.model_path = comp_json["model_path"];

                if (comp_json.contains("flip_textures") && comp_json["flip_textures"].is_boolean())
                    r.flip_textures = comp_json["flip_textures"];

                if (asset_mgr)
                    loadRenderableModel(r, device, *asset_mgr);
                else
                    loadRenderableModel(r, device);

                if (comp_json.contains("mesh_overrides") && comp_json["mesh_overrides"].is_array() && r.render_object)
                {
                    const auto& arr = comp_json["mesh_overrides"];
                    for (size_t i = 0; i < arr.size(); ++i)
                    {
                        auto* mesh = r.render_object->getMesh(i);
                        if (!mesh)
                            break;
                        auto&       mat   = mesh->m_material;
                        auto&       props = mat.properties;
                        const auto& m     = arr[i];

                        if (m.contains("shading_model") && m["shading_model"].is_number_integer())
                            mat.shading_model = static_cast<ShadingModel>(m["shading_model"].get<int>());
                        if (m.contains("blend_mode") && m["blend_mode"].is_number_integer())
                            mat.blend_mode = static_cast<BlendMode>(m["blend_mode"].get<int>());
                        if (m.contains("render_face") && m["render_face"].is_number_integer())
                            mat.render_face = static_cast<RenderFace>(m["render_face"].get<int>());
                        if (m.contains("alpha_cutoff") && m["alpha_cutoff"].is_number())
                            mat.alpha_cutoff = m["alpha_cutoff"];
                        if (m.contains("render_queue") && m["render_queue"].is_number_integer())
                            mat.render_queue = m["render_queue"];
                        if (m.contains("vert_path") && m["vert_path"].is_string())
                            mat.vert_path = m["vert_path"];
                        if (m.contains("frag_path") && m["frag_path"].is_string())
                            mat.frag_path = m["frag_path"];

                        if (m.contains("properties") && m["properties"].is_array())
                        {
                            for (const auto& pj : m["properties"])
                            {
                                if (!pj.contains("name") || !pj["name"].is_string())
                                    continue;
                                std::string pname = pj["name"];
                                auto        ptype = static_cast<PropType>(pj.value("type", 0));
                                float       v[4]  = {};
                                if (pj.contains("values") && pj["values"].is_array() && pj["values"].size() >= 4)
                                {
                                    v[0] = pj["values"][0];
                                    v[1] = pj["values"][1];
                                    v[2] = pj["values"][2];
                                    v[3] = pj["values"][3];
                                }
                                switch (ptype)
                                {
                                    case PropType::Bool:
                                        props.setBool(pname, v[0] != 0.0f);
                                        break;
                                    case PropType::Float:
                                        props.setFloat(pname, v[0]);
                                        break;
                                    case PropType::Int:
                                        props.setInt(pname, static_cast<int>(v[0]));
                                        break;
                                    case PropType::Vec2:
                                        props.setVec2(pname, glm::vec2(v[0], v[1]));
                                        break;
                                    case PropType::Vec3:
                                        props.setVec3(pname, glm::vec3(v[0], v[1], v[2]));
                                        break;
                                    case PropType::Vec4:
                                        props.setVec4(pname, glm::vec4(v[0], v[1], v[2], v[3]));
                                        break;
                                    case PropType::Texture2D:
                                        break;
                                }
                            }
                        }

                        props.setFloat("material.alphaCutout", mat.alpha_cutoff);

                        if (m.contains("opacity") && m["opacity"].is_number() && !m.contains("properties"))
                        {
                            props.setFloat("material.opacity", m["opacity"]);
                            if (m.contains("is_transparent") && m["is_transparent"].get<bool>())
                                mat.blend_mode = BlendMode::Transparent;
                            if (m.contains("double_sided") && m["double_sided"].get<bool>())
                                mat.render_face = RenderFace::Both;
                            if (m.contains("alpha_cutout") && m["alpha_cutout"].is_number())
                                mat.alpha_cutoff = m["alpha_cutout"];
                            if (m.contains("subsurface_enabled"))
                                props.setBool("material.subsurfaceEnabled", m["subsurface_enabled"]);
                            if (m.contains("subsurface_radius"))
                                props.setFloat("material.subsurfaceRadius", m["subsurface_radius"]);
                            if (m.contains("subsurface_color") && m["subsurface_color"].is_array() &&
                                m["subsurface_color"].size() == 3)
                                props.setVec3("material.subsurfaceColor",
                                              glm::vec3(m["subsurface_color"][0],
                                                        m["subsurface_color"][1],
                                                        m["subsurface_color"][2]));
                            if (m.contains("emissive") && m["emissive"].is_array() && m["emissive"].size() == 3)
                                props.setVec3("material.emissive",
                                              glm::vec3(m["emissive"][0], m["emissive"][1], m["emissive"][2]));
                            if (m.contains("emissive_strength"))
                                props.setFloat("material.emissiveStrength", m["emissive_strength"]);
                            if (m.contains("use_custom_shader") && m["use_custom_shader"].get<bool>())
                            {
                                if (m.contains("custom_vert_path"))
                                    mat.vert_path = m["custom_vert_path"];
                                if (m.contains("custom_frag_path"))
                                    mat.frag_path = m["custom_frag_path"];
                            }
                        }
                    }
                }
            }
            else if (type == "Point")
            {
                auto& [color, intensity, enabled, constant, linear, quadratic, range] = entity.emplace<PointLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    enabled = comp_json["enabled"];
                if (comp_json.contains("range") && comp_json["range"].is_number())
                    range = comp_json["range"];
                if (comp_json.contains("constant") && comp_json["constant"].is_number())
                    constant = comp_json["constant"];
                if (comp_json.contains("linear") && comp_json["linear"].is_number())
                    linear = comp_json["linear"];
                if (comp_json.contains("quadratic") && comp_json["quadratic"].is_number())
                    quadratic = comp_json["quadratic"];
            }
            else if (type == "Spot")
            {
                auto& [color,
                       intensity,
                       enabled,
                       constant,
                       linear,
                       quadratic,
                       range,
                       inner_cone_angle,
                       outer_cone_angle] = entity.emplace<SpotLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    enabled = comp_json["enabled"];
                if (comp_json.contains("range") && comp_json["range"].is_number())
                    range = comp_json["range"];
                if (comp_json.contains("constant") && comp_json["constant"].is_number())
                    constant = comp_json["constant"];
                if (comp_json.contains("linear") && comp_json["linear"].is_number())
                    linear = comp_json["linear"];
                if (comp_json.contains("quadratic") && comp_json["quadratic"].is_number())
                    quadratic = comp_json["quadratic"];
                if (comp_json.contains("inner_cone_angle") && comp_json["inner_cone_angle"].is_number())
                    inner_cone_angle = comp_json["inner_cone_angle"];
                if (comp_json.contains("outer_cone_angle") && comp_json["outer_cone_angle"].is_number())
                    outer_cone_angle = comp_json["outer_cone_angle"];
            }
            else if (type == "Directional")
            {
                auto& [color, intensity, enabled] = entity.emplace<DirectionalLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    enabled = comp_json["enabled"];
            }
            else if (type == "Area")
            {
                auto& [color, intensity, enabled, width, height] = entity.emplace<AreaLight>();

                if (comp_json.contains("color") && comp_json["color"].is_array() && comp_json["color"].size() == 3)
                    color = glm::vec3(comp_json["color"][0], comp_json["color"][1], comp_json["color"][2]);
                if (comp_json.contains("intensity") && comp_json["intensity"].is_number())
                    intensity = comp_json["intensity"];
                if (comp_json.contains("enabled") && comp_json["enabled"].is_boolean())
                    enabled = comp_json["enabled"];
                if (comp_json.contains("width") && comp_json["width"].is_number())
                    width = comp_json["width"];
                if (comp_json.contains("height") && comp_json["height"].is_number())
                    height = comp_json["height"];
            }
            else if (type == "AudioSource")
            {
                auto& [clip_path, volume, loop, spatial, play_on_start, playing, start_attempted] =
                    entity.emplace<AudioSource>();

                if (comp_json.contains("clip_path") && comp_json["clip_path"].is_string())
                    clip_path = comp_json["clip_path"];
                if (comp_json.contains("volume") && comp_json["volume"].is_number())
                    volume = comp_json["volume"];
                if (comp_json.contains("loop") && comp_json["loop"].is_boolean())
                    loop = comp_json["loop"];
                if (comp_json.contains("spatial") && comp_json["spatial"].is_boolean())
                    spatial = comp_json["spatial"];
                if (comp_json.contains("play_on_start") && comp_json["play_on_start"].is_boolean())
                    play_on_start = comp_json["play_on_start"];
            }
            else if (type == "AudioListener")
            {
                auto& primary = entity.emplace<AudioListener>().primary;
                if (comp_json.contains("primary") && comp_json["primary"].is_boolean())
                    primary = comp_json["primary"];
            }
        }
    }

} // namespace RealmEngine
