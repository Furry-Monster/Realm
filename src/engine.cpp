#include "engine.h"
#include "config_manager.h"
#include "gameplay/scene.h"
#include "global_context.h"
#include "render/render_entity.h"
#include "render/render_model.h"
#include "render/render_scene.h"
#include "render/renderer.h"
#include "utils.h"
#include "window.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <utility>

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

        std::string model_path = g_context.m_cfg->getAssetFolder().generic_string() + "/helmet/DamagedHelmet.gltf";
        try
        {
            // Don't flip textures for glTF
            auto model  = std::make_shared<RenderModel>(model_path, false);
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

        info("Starting render loop for helmet model...");

        while (!g_context.m_window->shouldClose() && frame_count < 600)
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

        g_context.destroy();
    }

    void Engine::tick()
    {
        logicalTick(m_scene);
        renderTick(m_render_scene);
    }

    void Engine::logicalTick(std::shared_ptr<Scene> scene)
    {
        if (scene == nullptr)
        {
            err("Logical Ticking failed due to the missing logical scene data.");
            return;
        }

        g_context.m_window->pollEvents();
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
