#include "engine.h"
#include "config_manager.h"
#include "gameplay/entity.h"
#include "gameplay/scene.h"
#include "global_context.h"
#include "render/render_model.h"
#include "render/renderer.h"
#include "utils.h"
#include "window.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

namespace RealmEngine
{
    void Engine::boot()
    {
        g_context.create();

        // Initialize renderer with window
        g_context.m_renderer->initialize(g_context.m_window);

        info("<<< Boot Engine Done. >>>");
    }

    void Engine::debugRun()
    {
        int frame_count = 0;

        // Create a simple scene
        auto scene = std::make_shared<Scene>();

        // Add some test lights
        scene->m_light_positions.push_back(glm::vec3(0.0f, 0.0f, 10.0f));
        scene->m_light_colors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

        // Try to load helmet model (glTF format)
        std::string model_path = g_context.m_cfg->getAssetFolder().generic_string() + "/helmet/DamagedHelmet.gltf";
        try
        {
            auto model  = std::make_shared<RenderModel>(model_path);
            auto entity = Entity(model);
            entity.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            entity.setScale(glm::vec3(1.0f, 1.0f, 1.0f)); // Model is already in correct scale
            scene->m_entities.push_back(entity);
            info("Loaded model: " + model_path);
        }
        catch (...)
        {
            warn("Failed to load model: " + model_path + ", continuing without it");
        }

        // Set camera position - adjust for helmet model
        // Helmet model is roughly 2 units in size, so position camera at appropriate distance
        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        // debug rendering, without game logic
        while (!g_context.m_window->shouldClose() && frame_count < 600)
        {
            tick(scene);
            frame_count++;

            if (frame_count % 60 == 0)
                debug("Rendered " + std::to_string(frame_count) + " frames");
        }

        debug("Render loop completed. Total frames: " + std::to_string(frame_count));
    }

    void Engine::run()
    {
        auto scene = std::make_shared<Scene>();

        // Add some test lights
        scene->m_light_positions.push_back(glm::vec3(0.0f, 0.0f, 10.0f));
        scene->m_light_colors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

        while (!g_context.m_window->shouldClose())
            tick(scene);
    }

    void Engine::terminate()
    {
        info("<<< Now Terminating Engine. >>>");
        g_context.m_renderer->shutdown();
        g_context.destroy();
    }

    void Engine::tick(std::shared_ptr<Scene> scene)
    {
        logicalTick();
        renderTick(scene);
    }

    void Engine::logicalTick() { g_context.m_window->pollEvents(); }

    void Engine::renderTick(std::shared_ptr<Scene> scene)
    {
        g_context.m_renderer->render(scene);
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
