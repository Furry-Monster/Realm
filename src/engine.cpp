#include "engine.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>

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

        info("<<< Boot Engine Done. >>>");
    }

    void Engine::run()
    {

        std::filesystem::path scene_file =
            g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

        if (std::filesystem::exists(scene_file))
        {
            info("Loading scene from: " + scene_file.string());
            m_scene = SceneSerializer::loadFromFile(scene_file.string());
        }
        if (!m_scene)
        {
            info("Loading failed, creat default scene instead.");
            m_scene = createDefaultScene();
        }

        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        m_scene->setCamera(g_context.m_renderer->getCamera());

        m_last_frame_time = glfwGetTime();
        while (!g_context.m_window->shouldClose())
            tick();

        info("Saving scene to: " + scene_file.string());
        if (SceneSerializer::saveToFile(m_scene, scene_file.string()))
            debug("Scene saved successfully.");
        else
            err("Failed to save scene file.");
    }

    std::shared_ptr<Scene> Engine::createDefaultScene()
    {
        auto scene = std::make_shared<Scene>();

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
        g_context.m_window->swapBuffer();
    }

} // namespace RealmEngine
