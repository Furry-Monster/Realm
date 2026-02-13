#include "renderer/render_scene.h"

#include "scene/components/lighting/area.h"
#include "scene/components/lighting/directional.h"
#include "scene/components/lighting/point.h"
#include "scene/components/lighting/spot.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/scene.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    void RenderScene::syncFromScene(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return;

        const bool scene_changed     = (scene != m_cached_scene);
        const bool structure_changed = scene_changed || (scene->getGeneration() != m_cached_generation);

        if (structure_changed)
        {
            fullSync(*scene);
            m_cached_scene      = scene;
            m_cached_generation = scene->getGeneration();
        }
        else
        {
            updateTransformsOnly(*scene);
        }
    }

    void RenderScene::fullSync(Scene& scene)
    {
        m_render_objects.clear();
        m_lights.clear();
        m_render_entities.clear();
        m_light_entities.clear();

        auto root = scene.getRoot();
        if (root)
            syncNode(scene, root);
    }

    void RenderScene::syncNode(Scene& scene, std::shared_ptr<SceneNode> node)
    {
        if (!node)
            return;

        if (node->hasEntity())
        {
            entt::entity entity = node->getEntity();
            if (!scene.valid(entity))
            {
                node->forEachChild([this, &scene](std::shared_ptr<SceneNode> child) { syncNode(scene, child); });
                return;
            }

            Transform tf;
            if (auto* t = scene.tryGet<Transform>(entity))
                tf = *t;

            if (auto* r = scene.tryGet<Renderable>(entity))
            {
                if (r->render_object)
                {
                    r->render_object->setPosition(tf.position);
                    r->render_object->setOrientation(tf.rotation);
                    r->render_object->setScale(tf.scale);
                    m_render_objects.push_back(r->render_object);
                    m_render_entities.push_back(entity);
                }
            }

            if (auto* pl = scene.tryGet<PointLight>(entity))
            {
                if (pl->enabled)
                {
                    Light light {};
                    light.type             = LightType::Point;
                    light.position         = tf.position;
                    light.direction        = glm::vec3(0.0f);
                    light.color            = pl->color;
                    light.intensity        = pl->intensity;
                    light.constant         = pl->constant;
                    light.linear           = pl->linear;
                    light.quadratic        = pl->quadratic;
                    light.range            = pl->range;
                    light.inner_cone_angle = 0.0f;
                    light.outer_cone_angle = 0.0f;
                    light.width            = 0.0f;
                    light.height           = 0.0f;
                    m_lights.push_back(light);
                    m_light_entities.push_back(entity);
                }
            }

            if (auto* sl = scene.tryGet<SpotLight>(entity))
            {
                if (sl->enabled)
                {
                    Light light {};
                    light.type             = LightType::Spot;
                    light.position         = tf.position;
                    light.direction        = -tf.getForward();
                    light.color            = sl->color;
                    light.intensity        = sl->intensity;
                    light.constant         = sl->constant;
                    light.linear           = sl->linear;
                    light.quadratic        = sl->quadratic;
                    light.range            = sl->range;
                    light.inner_cone_angle = sl->inner_cone_angle;
                    light.outer_cone_angle = sl->outer_cone_angle;
                    light.width            = 0.0f;
                    light.height           = 0.0f;
                    m_lights.push_back(light);
                    m_light_entities.push_back(entity);
                }
            }

            if (auto* al = scene.tryGet<AreaLight>(entity))
            {
                if (al->enabled)
                {
                    Light light {};
                    light.type             = LightType::Area;
                    light.position         = tf.position;
                    light.direction        = -tf.getForward();
                    light.color            = al->color;
                    light.intensity        = al->intensity;
                    light.constant         = 0.0f;
                    light.linear           = 0.0f;
                    light.quadratic        = 0.0f;
                    light.range            = 0.0f;
                    light.inner_cone_angle = 0.0f;
                    light.outer_cone_angle = 0.0f;
                    light.width            = al->width;
                    light.height           = al->height;
                    m_lights.push_back(light);
                    m_light_entities.push_back(entity);
                }
            }

            if (auto* dl = scene.tryGet<DirectionalLight>(entity))
            {
                if (dl->enabled)
                {
                    Light light {};
                    light.type             = LightType::Directional;
                    light.position         = glm::vec3(0.0f);
                    light.direction        = -tf.getForward();
                    light.color            = dl->color;
                    light.intensity        = dl->intensity;
                    light.constant         = 0.0f;
                    light.linear           = 0.0f;
                    light.quadratic        = 0.0f;
                    light.range            = 0.0f;
                    light.inner_cone_angle = 0.0f;
                    light.outer_cone_angle = 0.0f;
                    light.width            = 0.0f;
                    light.height           = 0.0f;
                    m_lights.push_back(light);
                    m_light_entities.push_back(entity);
                }
            }
        }

        node->forEachChild([this, &scene](std::shared_ptr<SceneNode> child) { syncNode(scene, child); });
    }

    void RenderScene::updateTransformsOnly(Scene& scene)
    {
        for (size_t i = 0; i < m_render_entities.size(); ++i)
        {
            entt::entity entity = m_render_entities[i];
            if (!scene.valid(entity) || i >= m_render_objects.size())
                continue;

            Transform tf;
            if (auto* t = scene.tryGet<Transform>(entity))
                tf = *t;

            m_render_objects[i]->setPosition(tf.position);
            m_render_objects[i]->setOrientation(tf.rotation);
            m_render_objects[i]->setScale(tf.scale);
        }

        for (size_t i = 0; i < m_light_entities.size(); ++i)
        {
            entt::entity entity = m_light_entities[i];
            if (!scene.valid(entity) || i >= m_lights.size())
                continue;

            Transform tf;
            if (auto* t = scene.tryGet<Transform>(entity))
                tf = *t;

            Light& light = m_lights[i];

            if (auto* pl = scene.tryGet<PointLight>(entity))
            {
                if (pl->enabled)
                {
                    light.position  = tf.position;
                    light.color     = pl->color;
                    light.intensity = pl->intensity;
                    light.constant  = pl->constant;
                    light.linear    = pl->linear;
                    light.quadratic = pl->quadratic;
                    light.range     = pl->range;
                }
            }
            else if (auto* sl = scene.tryGet<SpotLight>(entity))
            {
                if (sl->enabled)
                {
                    light.position         = tf.position;
                    light.direction        = -tf.getForward();
                    light.color            = sl->color;
                    light.intensity        = sl->intensity;
                    light.constant         = sl->constant;
                    light.linear           = sl->linear;
                    light.quadratic        = sl->quadratic;
                    light.range            = sl->range;
                    light.inner_cone_angle = sl->inner_cone_angle;
                    light.outer_cone_angle = sl->outer_cone_angle;
                }
            }
            else if (auto* al = scene.tryGet<AreaLight>(entity))
            {
                if (al->enabled)
                {
                    light.position  = tf.position;
                    light.direction = -tf.getForward();
                    light.color     = al->color;
                    light.intensity = al->intensity;
                    light.width     = al->width;
                    light.height    = al->height;
                }
            }
            else if (auto* dl = scene.tryGet<DirectionalLight>(entity))
            {
                if (dl->enabled)
                {
                    light.direction = -tf.getForward();
                    light.color     = dl->color;
                    light.intensity = dl->intensity;
                }
            }
        }
    }

} // namespace RealmEngine
