#include "scene/scene_manager.h"

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/log/log_macros.h"
#include "scene/components/lighting/area.h"
#include "scene/components/lighting/directional.h"
#include "scene/components/lighting/point.h"
#include "scene/components/lighting/spot.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/serialization/scene_serializer.h"

namespace RealmEngine
{
    void SceneManager::initialize(const std::filesystem::path& asset_folder) { m_asset_folder = asset_folder; }

    std::shared_ptr<Scene> SceneManager::createScene(const std::string& name)
    {
        if (m_scenes.find(name) != m_scenes.end())
            return m_scenes[name];

        auto scene     = std::make_shared<Scene>();
        m_scenes[name] = scene;
        return scene;
    }

    std::shared_ptr<Scene> SceneManager::createDefaultScene()
    {
        auto scene      = std::make_shared<Scene>();
        auto asset_path = m_asset_folder.generic_string();

        try
        {
            auto helmet_node   = scene->createNodeWithEntity("Helmet");
            auto helmet_entity = scene->findEntity("Helmet");

            auto& transform1    = scene->emplace<Transform>(helmet_entity);
            transform1.position = glm::vec3(0.0f, 0.0f, 0.0f);
            transform1.rotation = glm::angleAxis(glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));

            std::string helmet_path = asset_path + "/helmet/DamagedHelmet.gltf";
            auto&       renderable1 = scene->emplace<Renderable>(helmet_entity);
            renderable1.model_path  = helmet_path;
            renderable1.loadModel();

            scene->getRoot()->addChild(helmet_node);
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to load helmet model: " + std::string(e.what()));
        }

        try
        {
            auto sphere_node   = scene->createNodeWithEntity("Sphere");
            auto sphere_entity = scene->findEntity("Sphere");

            auto& transform2    = scene->emplace<Transform>(sphere_entity);
            transform2.position = glm::vec3(1.0f, 2.0f, 0.0f);

            std::string sphere_path = asset_path + "/sphere/sphere.gltf";
            auto&       renderable2 = scene->emplace<Renderable>(sphere_entity);
            renderable2.model_path  = sphere_path;
            renderable2.loadModel();

            scene->getRoot()->addChild(sphere_node);
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to load sphere model: " + std::string(e.what()));
        }

        // Point light 1 - main white light from above
        {
            auto  light1_node   = scene->createNodeWithEntity("PointLight1");
            auto  light1_entity = scene->findEntity("PointLight1");
            auto& t             = scene->emplace<Transform>(light1_entity);
            t.position          = glm::vec3(0.0f, 10.0f, 0.0f);
            auto& pl            = scene->emplace<PointLight>(light1_entity);
            pl.color            = glm::vec3(200.0f, 200.0f, 200.0f);
            pl.intensity        = 1.0f;
            pl.range            = 50.0f;
            scene->getRoot()->addChild(light1_node);
        }

        // Point light 2 - red light from left
        {
            auto  light2_node   = scene->createNodeWithEntity("PointLight2");
            auto  light2_entity = scene->findEntity("PointLight2");
            auto& t             = scene->emplace<Transform>(light2_entity);
            t.position          = glm::vec3(-5.0f, 3.0f, 0.0f);
            auto& pl            = scene->emplace<PointLight>(light2_entity);
            pl.color            = glm::vec3(200.0f, 50.0f, 50.0f);
            pl.intensity        = 0.8f;
            pl.range            = 30.0f;
            scene->getRoot()->addChild(light2_node);
        }

        // Point light 3 - blue light from right
        {
            auto  light3_node   = scene->createNodeWithEntity("PointLight3");
            auto  light3_entity = scene->findEntity("PointLight3");
            auto& t             = scene->emplace<Transform>(light3_entity);
            t.position          = glm::vec3(5.0f, 3.0f, 0.0f);
            auto& pl            = scene->emplace<PointLight>(light3_entity);
            pl.color            = glm::vec3(50.0f, 50.0f, 200.0f);
            pl.intensity        = 0.8f;
            pl.range            = 30.0f;
            scene->getRoot()->addChild(light3_node);
        }

        // Directional light - sun-like from top-right
        {
            auto  dir_node            = scene->createNodeWithEntity("DirectionalLight");
            auto  dir_entity          = scene->findEntity("DirectionalLight");
            auto& t                   = scene->emplace<Transform>(dir_entity);
            t.position                = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 dir_forward     = glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f));
            glm::vec3 default_forward = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 axis            = glm::cross(default_forward, dir_forward);
            float     angle           = glm::acos(glm::dot(default_forward, dir_forward));
            if (glm::length(axis) > 0.001f)
                t.rotation = glm::angleAxis(angle, glm::normalize(axis));
            auto& dl     = scene->emplace<DirectionalLight>(dir_entity);
            dl.color     = glm::vec3(255.0f, 250.0f, 200.0f);
            dl.intensity = 0.5f;
            scene->getRoot()->addChild(dir_node);
        }

        // Spot light - focused from front
        {
            auto      spot_node       = scene->createNodeWithEntity("SpotLight");
            auto      spot_entity     = scene->findEntity("SpotLight");
            auto&     t               = scene->emplace<Transform>(spot_entity);
            glm::vec3 spot_position   = glm::vec3(0.0f, 5.0f, 8.0f);
            t.position                = spot_position;
            glm::vec3 default_forward = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 spot_target     = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 spot_forward    = glm::normalize(spot_target - spot_position);
            glm::vec3 spot_axis       = glm::cross(default_forward, spot_forward);
            float     spot_angle      = glm::acos(glm::dot(default_forward, spot_forward));
            if (glm::length(spot_axis) > 0.001f)
                t.rotation = glm::angleAxis(spot_angle, glm::normalize(spot_axis));
            auto& sl            = scene->emplace<SpotLight>(spot_entity);
            sl.color            = glm::vec3(200.0f, 200.0f, 150.0f);
            sl.intensity        = 1.2f;
            sl.range            = 20.0f;
            sl.inner_cone_angle = 15.0f;
            sl.outer_cone_angle = 30.0f;
            scene->getRoot()->addChild(spot_node);
        }

        return scene;
    }

    std::shared_ptr<Scene> SceneManager::loadScene(const std::string& filepath)
    {
        auto scene = SceneSerializer::loadFromFile(filepath);
        if (scene)
        {
            std::string name = std::filesystem::path(filepath).stem().string();
            m_scenes[name]   = scene;
        }
        return scene;
    }

    bool SceneManager::saveScene(const std::string& name, const std::string& filepath)
    {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return false;

        return SceneSerializer::saveToFile(it->second, filepath);
    }

    bool SceneManager::saveCurrentScene(const std::string& filepath)
    {
        if (!m_current_scene)
            return false;

        return SceneSerializer::saveToFile(m_current_scene, filepath);
    }

    void SceneManager::setCurrentScene(const std::string& name)
    {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return;

        setCurrentScene(it->second);
    }

    void SceneManager::setCurrentScene(std::shared_ptr<Scene> scene)
    {
        if (m_current_scene == scene)
            return;

        auto old_scene  = m_current_scene;
        m_current_scene = scene;

        if (m_on_scene_changed)
            m_on_scene_changed(old_scene, m_current_scene);
    }

    std::shared_ptr<Scene> SceneManager::getCurrentScene() const { return m_current_scene; }

    std::shared_ptr<Scene> SceneManager::getCurrentOrNewScene()
    {
        if (!m_current_scene)
            setCurrentScene(createScene("Unsaved"));

        return m_current_scene;
    }

    std::shared_ptr<Scene> SceneManager::getScene(const std::string& name) const
    {
        auto it = m_scenes.find(name);
        return (it != m_scenes.end()) ? it->second : nullptr;
    }

    bool SceneManager::hasScene(const std::string& name) const { return m_scenes.find(name) != m_scenes.end(); }

    void SceneManager::removeScene(const std::string& name)
    {
        if (m_current_scene && m_scenes.find(name) != m_scenes.end() && m_scenes[name] == m_current_scene)
            m_current_scene = nullptr;

        m_scenes.erase(name);
    }

    void SceneManager::setOnSceneChanged(SceneChangeCallback callback) { m_on_scene_changed = callback; }

} // namespace RealmEngine
