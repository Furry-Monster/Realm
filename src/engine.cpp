#include "engine.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <filesystem>
#include <memory>
#include <string>

#include "gameplay/components/lighting.h"
#include "gameplay/components/renderable.h"
#include "gameplay/components/transform.h"
#include "gameplay/scene/scene_serializer.h"
#include "global_context.h"
#include "input.h"
#include "render/render_scene.h"
#include "render/renderer.h"
#include "resource/config_manager.h"
#include "utils.h"
#include "window.h"

namespace RealmEngine
{
    void Engine::boot()
    {
        g_context.create();

        const GamePlayConfig& gameplay_config = g_context.m_config->getGamePlayConfig();
        m_max_delta_time                      = gameplay_config.max_delta_time;

        std::filesystem::path scene_file =
            g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

        if (std::filesystem::exists(scene_file))
        {
            info("Loading scene from: " + scene_file.string());
            m_scene = SceneSerializer::loadFromFile(scene_file.string());
        }

        if (!m_scene)
        {
            info("Loading failed, create default scene instead.");
            m_scene = createDefaultScene();
        }

        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        m_last_frame_time = glfwGetTime();

        info("<<< Boot Engine Done. >>>");
    }

    void Engine::debug()
    {
        info("<<< Run in Debug-Mode. >>>");

        while (!g_context.m_window->shouldClose())
        {
            tick();
            g_context.m_window->swapBuffer();
        }
    }

    std::shared_ptr<Scene> Engine::createDefaultScene()
    {
        auto scene = std::make_shared<Scene>();

        // Models
        std::string asset_path = g_context.m_config->getAssetFolder().generic_string();
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

        // lights
        auto light_node   = scene->createNodeWithEntity("Light");
        auto light_entity = scene->getEntity("Light");

        auto light_transform = std::make_shared<Transform>();
        light_transform->setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        light_entity->addComponent(light_transform);

        auto lighting = std::make_shared<Lighting>(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(200.0f, 200.0f, 200.0f));
        light_entity->addComponent(lighting);

        scene->getRoot()->addChild(light_node);

        return scene;
    }

    void Engine::terminate()
    {
        info("<<< Now Terminating Engine. >>>");

        if (m_scene)
        {
            std::filesystem::path scene_file =
                g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

            info("Saving scene to: " + scene_file.string());
            if (SceneSerializer::saveToFile(m_scene, scene_file.string()))
                info("Scene saved successfully.");
            else
                err("Failed to save scene file.");
        }

        m_delta_time = 0.0f;
        g_context.destroy();
    }

    void Engine::tick()
    {
        // tick timer first
        double current_time = glfwGetTime();
        m_delta_time        = static_cast<float>(current_time - m_last_frame_time);
        m_last_frame_time   = current_time;
        if (m_delta_time > m_max_delta_time)
            m_delta_time = m_max_delta_time;

        logicalTick(m_scene);
        renderTick();
    }

    void Engine::logicalTick(std::shared_ptr<Scene> scene) const
    {
        if (scene == nullptr)
        {
            err("Logical Ticking failed due to the missing logical scene data.");
            return;
        }

        g_context.m_input->tick();
        g_context.m_window->pollEvents();
        scene->tick(m_delta_time);
    }

    void Engine::renderTick()
    {
        g_context.m_renderer->getRenderScene()->syncFromScene(m_scene);
        g_context.m_renderer->render();
    }

} // namespace RealmEngine
