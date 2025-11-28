#include "gameplay/scene/scene_manager.h"

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "gameplay/components/lighting/point.h"
#include "gameplay/components/renderable.h"
#include "gameplay/components/transform.h"
#include "gameplay/scene/scene_serializer.h"
#include "global_context.h"
#include "resource/config_manager.h"
#include "utils.h"

namespace RealmEngine
{
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
        auto asset_path = g_context.m_config->getAssetFolder().generic_string();

        try
        {
            auto helmet_node   = scene->createNodeWithEntity("Helmet");
            auto helmet_entity = scene->getEntity("Helmet");

            auto transform1 = std::make_shared<Transform>();
            transform1->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            transform1->setRotation(glm::angleAxis(1.5708f, glm::vec3(1.0f, 0.0f, 0.0f)));
            helmet_entity->addComponent(transform1);

            std::string helmet_path = asset_path + "/helmet/DamagedHelmet.gltf";
            auto        renderable1 = std::make_shared<Renderable>(helmet_path, false);
            helmet_entity->addComponent(renderable1);

            scene->getRoot()->addChild(helmet_node);
        }
        catch (const std::exception& e)
        {
            err("Failed to load helmet model: " + std::string(e.what()));
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
            err("Failed to load sphere model: " + std::string(e.what()));
        }

        auto light_node   = scene->createNodeWithEntity("Light");
        auto light_entity = scene->getEntity("Light");

        auto light_transform = std::make_shared<Transform>();
        light_transform->setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        light_entity->addComponent(light_transform);

        auto point_light = std::make_shared<Point>();
        point_light->setColor(glm::vec3(200.0f, 200.0f, 200.0f));
        point_light->setIntensity(1.0f);
        light_entity->addComponent(point_light);

        scene->getRoot()->addChild(light_node);

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
