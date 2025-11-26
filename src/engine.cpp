#include "engine.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include "config_manager.h"
#include "gameplay/lighting.h"
#include "gameplay/renderable.h"
#include "gameplay/scene.h"
#include "gameplay/transform.h"
#include "global_context.h"
#include "input.h"
#include "render/render_object.h"
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

        auto scene        = std::make_shared<Scene>();
        auto render_scene = std::make_shared<RenderScene>();

        m_scene        = std::move(scene);
        m_render_scene = std::move(render_scene);

        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        m_scene->setCamera(g_context.m_renderer->getCamera());

        std::string asset_path = g_context.m_config->getAssetFolder().generic_string();

        try
        {
            auto helmet_node   = m_scene->createNodeWithEntity("Helmet");
            auto helmet_entity = m_scene->getEntity("Helmet");

            auto transform1 = std::make_unique<Transform>();
            transform1->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            transform1->setRotation(glm::angleAxis(1.5708f, glm::vec3(1.0f, 0.0f, 0.0f)));
            helmet_entity->addComponent(std::move(transform1));

            std::string helmet_path = asset_path + "/helmet/DamagedHelmet.gltf";
            auto        render_obj1 = std::make_shared<RenderObject>(helmet_path, false);
            auto        renderable1 = std::make_unique<Renderable>(render_obj1);
            helmet_entity->addComponent(std::move(renderable1));

            m_scene->getRoot()->addChild(helmet_node);
        }
        catch (const std::exception& e)
        {
            err("Failed to load helmet model: " + std::string(e.what()));
        }

        try
        {
            auto sphere_node   = m_scene->createNodeWithEntity("Sphere");
            auto sphere_entity = m_scene->getEntity("Sphere");

            auto transform2 = std::make_unique<Transform>();
            transform2->setPosition(glm::vec3(1.0f, 2.0f, 0.0f));
            sphere_entity->addComponent(std::move(transform2));

            std::string sphere_path = asset_path + "/sphere/sphere.gltf";
            auto        render_obj2 = std::make_shared<RenderObject>(sphere_path, false);
            auto        renderable2 = std::make_unique<Renderable>(render_obj2);
            sphere_entity->addComponent(std::move(renderable2));

            m_scene->getRoot()->addChild(sphere_node);
        }
        catch (const std::exception& e)
        {
            err("Failed to load sphere model: " + std::string(e.what()));
        }

        auto light_node   = m_scene->createNodeWithEntity("Light");
        auto light_entity = m_scene->getEntity("Light");

        auto light_transform = std::make_unique<Transform>();
        light_transform->setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        light_entity->addComponent(std::move(light_transform));

        auto lighting = std::make_unique<Lighting>(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(200.0f, 200.0f, 200.0f));
        light_entity->addComponent(std::move(lighting));

        m_scene->getRoot()->addChild(light_node);

        m_last_frame_time = glfwGetTime();

        info("Starting render loop for helmet model...");

        while (!g_context.m_window->shouldClose())
        {
            tick();
            frame_count++;

            if (frame_count % 60 == 0)
                debug("Rendered " + std::to_string(frame_count) + " frames");
        }

        debug("Render loop completed. Total frames: " + std::to_string(frame_count));
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

    void Engine::renderTick(std::shared_ptr<RenderScene> scene)
    {
        if (scene == nullptr)
        {
            err("Render Ticking failed due to the missing render scene data.");
            return;
        }

        // 从Scene同步渲染信息
        if (m_scene)
        {
            scene->syncFromScene(m_scene);
        }

        g_context.m_renderer->render(scene);
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
