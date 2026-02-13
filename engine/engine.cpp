#include "engine.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <filesystem>
#include <memory>
#include <string>

#include "scene/scene_manager.h"
#include "global_context.h"
#include "platform/input/input.h"
#include "renderer/renderer.h"
#include "resource/config_manager.h"
#include "core/base/utils.h"
#include "platform/window/window.h"

namespace RealmEngine
{
    void Engine::boot()
    {
        g_context.create();

        const GamePlayConfig& gameplay_config = g_context.m_config->getGamePlayConfig();
        m_max_delta_time                      = gameplay_config.max_delta_time;

        m_last_frame_time = glfwGetTime();

        RE_LOG_INFO("<<< Boot Engine Done. >>>");
    }

    void Engine::debug()
    {
        std::filesystem::path scene_file =
            g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

        std::shared_ptr<Scene> loaded;
        if (std::filesystem::exists(scene_file))
        {
            RE_LOG_INFO("Loading scene from: " + scene_file.string());
            loaded = g_context.m_scene->loadScene(scene_file.string());
        }

        if (!loaded)
        {
            RE_LOG_INFO("Loading failed, create default scene instead.");
            loaded = g_context.m_scene->createDefaultScene();
        }

        g_context.m_scene->setCurrentScene(loaded);

        g_context.m_renderer->getCamera()->setPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        g_context.m_renderer->getCamera()->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        RE_LOG_INFO("<<< Run in Debug-Mode. >>>");

        while (!g_context.m_window->shouldClose())
        {
            tick();
            g_context.m_window->swapBuffer();
        }

        if (g_context.m_scene && g_context.m_scene->getCurrentScene())
        {
            std::filesystem::path scene_file =
                g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

            RE_LOG_INFO("Saving scene to: " + scene_file.string());
            if (g_context.m_scene->saveCurrentScene(scene_file.string()))
                RE_LOG_INFO("Scene saved successfully.");
            else
                RE_LOG_ERROR("Failed to save scene file.");
        }
    }

    void Engine::terminate()
    {
        RE_LOG_INFO("<<< Now Terminating Engine. >>>");

        m_delta_time = 0.0f;
        g_context.destroy();
    }

    void Engine::tick()
    {
        double current_time = glfwGetTime();
        m_delta_time        = current_time - m_last_frame_time;
        m_last_frame_time   = current_time;
        if (m_delta_time > m_max_delta_time)
            m_delta_time = m_max_delta_time;

        logicalTick();
        renderTick();
    }

    void Engine::logicalTick() const
    {
        g_context.m_input->tick();
        g_context.m_window->pollEvents();
        g_context.m_scene->getCurrentOrNewScene()->tick(m_delta_time);
    }

    void Engine::renderTick() const
    {
        g_context.m_renderer->getRenderScene()->syncFromCurrentScene();
        g_context.m_renderer->render();
    }

} // namespace RealmEngine
