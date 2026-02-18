#include "scene/scene_manager.h"

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "core/base/macros.h"
#include "resource/asset_manager.h"
#include "scene/components/lighting/area.h"
#include "scene/components/lighting/directional.h"
#include "scene/components/lighting/light_probe.h"
#include "scene/components/lighting/point.h"
#include "scene/components/lighting/spot.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/serialization/scene_serializer.h"

namespace RealmEngine
{
    void SceneManager::initialize(const std::filesystem::path& asset_folder) { m_asset_folder = asset_folder; }

    std::shared_ptr<Scene> SceneManager::createScene(const std::string& name)
    {
        if (m_scenes.find(name) != m_scenes.end())
            return m_scenes[name];

        auto scene     = std::make_shared<Scene>();
        m_scenes[name] = scene;
        return scene;
    }

    std::shared_ptr<Scene> SceneManager::createDefaultScene(RHIDevice& device) const
    {
        auto scene      = std::make_shared<Scene>();
        auto asset_path = m_asset_folder.generic_string();

        auto loadModelAt = [this, &scene, &device, asset_path](const std::string& name,
                                                               const std::string& rel_path,
                                                               const glm::vec3&   pos,
                                                               const glm::quat&   rot,
                                                               const glm::vec3&   scale) {
            try
            {
                const auto node   = scene->createNodeWithEntity(name);
                auto       entity = scene->findEntity(name);
                auto&      t      = entity.emplace<Transform>();
                t.position        = pos;
                t.rotation        = rot;
                t.scale           = scale;
                auto& r           = entity.emplace<Renderable>();
                r.model_path      = asset_path + "/" + rel_path;
                if (m_asset_mgr)
                    loadRenderableModel(r, device, *m_asset_mgr);
                else
                    loadRenderableModel(r, device);
                scene->getRoot()->addChild(node);
                return true;
            }
            catch (const std::exception& e)
            {
                RE_LOG_ERROR("Failed to load " + name + ": " + std::string(e.what()));
                return false;
            }
        };

        loadModelAt("Helmet",
                    "helmet/DamagedHelmet.gltf",
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::angleAxis(glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)),
                    glm::vec3(1.0f));

        loadModelAt("Cian",
                    "Cian/Cian.gltf",
                    glm::vec3(-3.5f, 0.0f, 0.0f),
                    glm::angleAxis(glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)),
                    glm::vec3(1.5f));

        auto setLightDir = [](Transform& t, const glm::vec3& forward) {
            constexpr glm::vec3 def_fwd(0.0f, 0.0f, -1.0f);
            const glm::vec3     axis = glm::cross(def_fwd, forward);
            if (glm::length(axis) > 0.001f)
                t.rotation = glm::angleAxis(glm::acos(glm::clamp(glm::dot(def_fwd, forward), -1.0f, 1.0f)),
                                            glm::normalize(axis));
        };

        {
            const auto n = scene->createNodeWithEntity("PointLight1");
            auto       e = scene->findEntity("PointLight1");
            auto&      t = e.emplace<Transform>();
            t.position   = glm::vec3(0.0f, 9.0f, 0.0f);

            auto& pl     = e.emplace<PointLight>();
            pl.color     = glm::vec3(1.0f, 1.0f, 1.0f);
            pl.intensity = 5.0f;
            pl.range     = 50.0f;

            scene->getRoot()->addChild(n);
        }

        {
            const auto n = scene->createNodeWithEntity("PointLight2");
            auto       e = scene->findEntity("PointLight2");
            auto&      t = e.emplace<Transform>();
            t.position   = glm::vec3(-5.0f, 4.0f, 2.0f);

            auto& pl     = e.emplace<PointLight>();
            pl.color     = glm::vec3(1.0f, 0.25f, 0.2f);
            pl.intensity = 5.0f;
            pl.range     = 35.0f;

            scene->getRoot()->addChild(n);
        }

        {
            const auto n = scene->createNodeWithEntity("PointLight3");
            auto       e = scene->findEntity("PointLight3");
            auto&      t = e.emplace<Transform>();
            t.position   = glm::vec3(5.0f, 4.0f, 2.0f);

            auto& pl     = e.emplace<PointLight>();
            pl.color     = glm::vec3(0.2f, 0.3f, 1.0f);
            pl.intensity = 5.0f;
            pl.range     = 35.0f;

            scene->getRoot()->addChild(n);
        }

        {
            const auto n = scene->createNodeWithEntity("DirectionalLight");
            auto       e = scene->findEntity("DirectionalLight");
            auto&      t = e.emplace<Transform>();
            setLightDir(t, glm::normalize(glm::vec3(-1.0f, -1.2f, -0.5f)));

            auto& dl     = e.emplace<DirectionalLight>();
            dl.color     = glm::vec3(1.0f, 0.98f, 0.88f);
            dl.intensity = 3.5f;

            scene->getRoot()->addChild(n);
        }

        {
            const auto n = scene->createNodeWithEntity("SpotLight");
            auto       e = scene->findEntity("SpotLight");
            auto&      t = e.emplace<Transform>();
            t.position   = glm::vec3(0.0f, 5.0f, 10.0f);
            setLightDir(t, glm::normalize(glm::vec3(0.0f, -0.5f, -1.0f)));

            auto& sl            = e.emplace<SpotLight>();
            sl.color            = glm::vec3(1.0f, 1.0f, 0.95f);
            sl.intensity        = 8.0f;
            sl.range            = 25.0f;
            sl.inner_cone_angle = 12.0f;
            sl.outer_cone_angle = 28.0f;

            scene->getRoot()->addChild(n);
        }

        {
            const auto n = scene->createNodeWithEntity("AreaLight");
            auto       e = scene->findEntity("AreaLight");
            auto&      t = e.emplace<Transform>();
            t.position   = glm::vec3(0.0f, 7.0f, 2.0f);
            setLightDir(t, glm::vec3(0.0f, -1.0f, 0.0f));

            auto& al     = e.emplace<AreaLight>();
            al.color     = glm::vec3(0.95f, 1.0f, 1.0f);
            al.intensity = 3.0f;
            al.width     = 2.5f;
            al.height    = 2.5f;

            scene->getRoot()->addChild(n);
        }

        {
            const auto n = scene->createNodeWithEntity("LightProbe");
            auto       e = scene->findEntity("LightProbe");
            auto&      t = e.emplace<Transform>();
            t.position   = glm::vec3(0.0f, 1.0f, 0.0f);

            auto& lp            = e.emplace<LightProbe>();
            lp.influence_radius = 15.0f;
            lp.needs_update     = true;

            scene->getRoot()->addChild(n);
        }

        return scene;
    }

    std::shared_ptr<Scene> SceneManager::loadScene(const std::string& filepath, RHIDevice& device)
    {
        auto scene = SceneSerializer::loadFromFile(filepath, device, m_asset_mgr);
        if (scene)
        {
            const std::string name = std::filesystem::path(filepath).stem().string();
            m_scenes[name]         = scene;
        }
        return scene;
    }

    bool SceneManager::saveScene(const std::string& name, const std::string& filepath)
    {
        const auto it = m_scenes.find(name);
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
        const auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return;

        setCurrentScene(it->second);
    }

    void SceneManager::setCurrentScene(const std::shared_ptr<Scene>& scene)
    {
        if (m_current_scene == scene)
            return;

        const auto old_scene = m_current_scene;
        m_current_scene      = scene;

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
        const auto it = m_scenes.find(name);
        return (it != m_scenes.end()) ? it->second : nullptr;
    }

    bool SceneManager::hasScene(const std::string& name) const { return m_scenes.find(name) != m_scenes.end(); }

    void SceneManager::removeScene(const std::string& name)
    {
        const auto it = m_scenes.find(name);
        if (it == m_scenes.end())
            return;

        const bool was_current = (m_current_scene && it->second == m_current_scene);
        const auto old_scene   = m_current_scene;

        if (was_current)
            m_current_scene = nullptr;

        m_scenes.erase(it);

        if (was_current && m_on_scene_changed)
            m_on_scene_changed(old_scene, m_current_scene);
    }

    void SceneManager::setOnSceneChanged(const SceneChangeCallback& callback) { m_on_scene_changed = callback; }

} // namespace RealmEngine
