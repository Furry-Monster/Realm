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
        // Creates a default scene with sample models and lights for testing
        auto scene      = std::make_shared<Scene>();
        auto asset_path = m_asset_folder.generic_string();

        try
        {
            auto helmet_node   = scene->createNodeWithEntity("Helmet");
            auto helmet_entity = scene->getEntity("Helmet");

            auto transform1 = std::make_shared<Transform>();
            transform1->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            transform1->setRotation(glm::angleAxis(glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));
            helmet_entity->addComponent(transform1);

            std::string helmet_path = asset_path + "/helmet/DamagedHelmet.gltf";
            auto        renderable1 = std::make_shared<Renderable>(helmet_path, false);
            helmet_entity->addComponent(renderable1);

            scene->getRoot()->addChild(helmet_node);
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to load helmet model: " + std::string(e.what()));
        }

        try
        {
            auto sphere_node   = scene->createNodeWithEntity("Sphere");
            auto sphere_entity = scene->getEntity("Sphere");

            auto transform2 = std::make_shared<Transform>();
            transform2->setPosition(glm::vec3(1.0f, 2.0f, 0.0f));
            sphere_entity->addComponent(transform2);

            std::string sphere_path = asset_path + "/sphere/sphere.gltf";
            auto        renderable2 = std::make_shared<Renderable>(sphere_path, false);
            sphere_entity->addComponent(renderable2);

            scene->getRoot()->addChild(sphere_node);
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to load sphere model: " + std::string(e.what()));
        }

        // Point light 1 - main white light from above
        auto light1_node      = scene->createNodeWithEntity("PointLight1");
        auto light1_entity    = scene->getEntity("PointLight1");
        auto light1_transform = std::make_shared<Transform>();
        light1_transform->setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        light1_entity->addComponent(light1_transform);
        auto point_light1 = std::make_shared<Point>();
        point_light1->setColor(glm::vec3(200.0f, 200.0f, 200.0f));
        point_light1->setIntensity(1.0f);
        point_light1->setRange(50.0f);
        light1_entity->addComponent(point_light1);
        scene->getRoot()->addChild(light1_node);

        // Point light 2 - red light from left
        auto light2_node      = scene->createNodeWithEntity("PointLight2");
        auto light2_entity    = scene->getEntity("PointLight2");
        auto light2_transform = std::make_shared<Transform>();
        light2_transform->setPosition(glm::vec3(-5.0f, 3.0f, 0.0f));
        light2_entity->addComponent(light2_transform);
        auto point_light2 = std::make_shared<Point>();
        point_light2->setColor(glm::vec3(200.0f, 50.0f, 50.0f));
        point_light2->setIntensity(0.8f);
        point_light2->setRange(30.0f);
        light2_entity->addComponent(point_light2);
        scene->getRoot()->addChild(light2_node);

        // Point light 3 - blue light from right
        auto light3_node      = scene->createNodeWithEntity("PointLight3");
        auto light3_entity    = scene->getEntity("PointLight3");
        auto light3_transform = std::make_shared<Transform>();
        light3_transform->setPosition(glm::vec3(5.0f, 3.0f, 0.0f));
        light3_entity->addComponent(light3_transform);
        auto point_light3 = std::make_shared<Point>();
        point_light3->setColor(glm::vec3(50.0f, 50.0f, 200.0f));
        point_light3->setIntensity(0.8f);
        point_light3->setRange(30.0f);
        light3_entity->addComponent(point_light3);
        scene->getRoot()->addChild(light3_node);

        // Directional light - sun-like from top-right
        auto dir_light_node      = scene->createNodeWithEntity("DirectionalLight");
        auto dir_light_entity    = scene->getEntity("DirectionalLight");
        auto dir_light_transform = std::make_shared<Transform>();
        dir_light_transform->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        // Rotate to point from top-right: forward should be normalize(vec3(-1, -1, 0))
        glm::vec3 dir_forward     = glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f));
        glm::vec3 default_forward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 axis            = glm::cross(default_forward, dir_forward);
        float     angle           = glm::acos(glm::dot(default_forward, dir_forward));
        if (glm::length(axis) > 0.001f)
            dir_light_transform->setRotation(glm::angleAxis(angle, glm::normalize(axis)));
        dir_light_entity->addComponent(dir_light_transform);
        auto directional_light = std::make_shared<Directional>();
        directional_light->setColor(glm::vec3(255.0f, 250.0f, 200.0f));
        directional_light->setIntensity(0.5f);
        dir_light_entity->addComponent(directional_light);
        scene->getRoot()->addChild(dir_light_node);

        // Spot light - focused from front
        auto      spot_light_node      = scene->createNodeWithEntity("SpotLight");
        auto      spot_light_entity    = scene->getEntity("SpotLight");
        auto      spot_light_transform = std::make_shared<Transform>();
        glm::vec3 spot_position        = glm::vec3(0.0f, 5.0f, 8.0f);
        spot_light_transform->setPosition(spot_position);
        // Point towards origin
        glm::vec3 spot_target  = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 spot_forward = glm::normalize(spot_target - spot_position);
        glm::vec3 spot_axis    = glm::cross(default_forward, spot_forward);
        float     spot_angle   = glm::acos(glm::dot(default_forward, spot_forward));
        if (glm::length(spot_axis) > 0.001f)
            spot_light_transform->setRotation(glm::angleAxis(spot_angle, glm::normalize(spot_axis)));
        spot_light_entity->addComponent(spot_light_transform);
        auto spot_light = std::make_shared<Spot>();
        spot_light->setColor(glm::vec3(200.0f, 200.0f, 150.0f));
        spot_light->setIntensity(1.2f);
        spot_light->setRange(20.0f);
        spot_light->setInnerConeAngle(15.0f);
        spot_light->setOuterConeAngle(30.0f);
        spot_light_entity->addComponent(spot_light);
        scene->getRoot()->addChild(spot_light_node);

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
