#include "engine.h"
#include "gameplay/components/lighting.h"
#include "gameplay/components/renderable.h"
#include "gameplay/components/transform.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include "config_manager.h"
#include "gameplay/scene.h"
#include "gameplay/scene_serializer.h"
#include "global_context.h"
#include "input.h"
#include "render/render_scene.h"
#include "render/renderer.h"
#include "utils.h"
#include "window.h"

namespace RealmEngine
{
    void Engine::boot()
    {
        g_context.create();

        info("<<< Boot Engine Done. >>>");
    }

    void Engine::run()
    {
        int frame_count = 0;

        m_render_scene                   = std::make_shared<RenderScene>();
        std::filesystem::path scene_file = g_context.m_config->getRootFolder() / "scene.json";

        if (std::filesystem::exists(scene_file))
        {
            info("Loading scene from: " + scene_file.string());
            m_scene = SceneSerializer::loadFromFile(scene_file.string());
            if (!m_scene)
            {
                warn("Failed to load scene file, creating default scene.");
                m_scene = createDefaultScene();
            }
            else
            {
                info("Scene loaded successfully.");
            }
        }
        else
        {
            info("Scene file not found, creating default scene.");
            m_scene = createDefaultScene();
        }

        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        m_scene->setCamera(g_context.m_renderer->getCamera());

        m_last_frame_time = glfwGetTime();

        info("Starting render loop...");

        while (!g_context.m_window->shouldClose())
        {
            tick();
            frame_count++;

            if (frame_count % 60 == 0)
                debug("Rendered " + std::to_string(frame_count) + " frames");
        }

        debug("Render loop completed. Total frames: " + std::to_string(frame_count));

        info("Saving scene to: " + scene_file.string());
        if (SceneSerializer::saveToFile(m_scene, scene_file.string()))
            info("Scene saved successfully.");
        else
            err("Failed to save scene file.");
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

        m_delta_time = 0.0f;

        g_context.destroy();
    }

    void Engine::tick()
    {
        // tick timer first
        double current_time = glfwGetTime();
        m_delta_time        = static_cast<float>(current_time - m_last_frame_time);
        m_last_frame_time   = current_time;
        if (m_delta_time > 0.1f)
            m_delta_time = 0.1f;

        logicalTick(m_scene);
        renderTick(m_render_scene);
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

    void Engine::renderTick(std::shared_ptr<RenderScene> render_scene)
    {
        if (render_scene == nullptr)
        {
            err("Render Ticking failed due to the missing render scene data.");
            return;
        }

        render_scene->syncFromScene(m_scene);

        g_context.m_renderer->render(render_scene);
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
