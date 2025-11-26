#include "engine.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include "config_manager.h"
#include "gameplay/scene.h"
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

        render_scene->m_light_positions.push_back(glm::vec3(0.0f, 10.0f, 0.0f));
        render_scene->m_light_colors.push_back(glm::vec3(200.0f, 200.0f, 200.0f));

        m_scene        = std::move(scene);
        m_render_scene = std::move(render_scene);

        auto& obj1 = addRenderObject("/helmet/DamagedHelmet.gltf");
        obj1.setPosition({1.0, 1.0, 1.0});
        auto& obj2 = addRenderObject("/sphere/sphere.gltf");
        obj2.setPosition({5.0, 5.0, 5.0});

        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        m_scene->setCamera(g_context.m_renderer->getCamera());

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

    RenderObject& Engine::addRenderObject(std::string path)
    {
        std::string model_path = g_context.m_config->getAssetFolder().generic_string() + path;
        try
        {
            // Don't flip textures for glTF
            auto& obj = m_render_scene->m_render_objects.emplace_back(model_path, false);

            obj.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            obj.setScale(glm::vec3(1.0f, 1.0f, 1.0f));
            obj.setOrientation(glm::angleAxis(1.5708f, glm::vec3(1.0f, 0.0f, 0.0f)));

            return obj;
        }
        catch (const std::exception& e)
        {
            err("Failed to load : " + model_path);
            fatal("Error: " + std::string(e.what()));
            throw std::runtime_error(std::string("Failed to load ") + model_path + ": " + e.what());
        }
        catch (...)
        {
            fatal("Failed to load : " + model_path + " (unknown error)");
            throw std::runtime_error(std::string("Failed to load ") + model_path + " (unknown error)");
        }
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

        g_context.m_renderer->render(scene);
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
