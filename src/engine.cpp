#include "engine.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "config_manager.h"
#include "gameplay/scene.h"
#include "global_context.h"
#include "input.h"
#include "render/render_entity.h"
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

    void Engine::debugRun()
    {
        int frame_count = 0;

        auto&& scene        = std::make_shared<Scene>();
        auto&& render_scene = std::make_shared<RenderScene>();

        render_scene->m_light_positions.push_back(glm::vec3(0.0f, 10.0f, 0.0f));
        render_scene->m_light_colors.push_back(glm::vec3(200.0f, 200.0f, 200.0f));

        m_scene        = scene;
        m_render_scene = render_scene;

        std::string model_path = g_context.m_config->getAssetFolder().generic_string() + "/helmet/DamagedHelmet.gltf";
        try
        {
            // Don't flip textures for glTF
            auto model  = std::make_shared<RenderObject>(model_path, false);
            auto entity = RenderEntity(model);
            entity.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            entity.setScale(glm::vec3(1.0f, 1.0f, 1.0f));

            entity.setOrientation(glm::angleAxis(1.5708f, glm::vec3(1.0f, 0.0f, 0.0f)));
            render_scene->m_entities.push_back(entity);
            info("Successfully loaded helmet model: " + model_path);
        }
        catch (const std::exception& e)
        {
            err("Failed to load helmet model: " + model_path);
            err("Error: " + std::string(e.what()));
        }
        catch (...)
        {
            err("Failed to load helmet model: " + model_path + " (unknown error)");
        }

        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        scene->initialize(g_context.m_renderer->getCamera());

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

    void Engine::run()
    {
        while (!g_context.m_window->shouldClose())
            tick();
    }

    void Engine::terminate()
    {
        info("<<< Now Terminating Engine. >>>");

        m_delta_time = 0.0f;

        g_context.destroy();
    }

    void Engine::tick()
    {
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

        g_context.m_renderer->render(scene);
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
