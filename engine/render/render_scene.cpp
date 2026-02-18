#include "render/render_scene.h"

#include "module/ecs/components/lighting/area.h"
#include "module/ecs/components/lighting/directional.h"
#include "module/ecs/components/lighting/point.h"
#include "module/ecs/components/lighting/spot.h"
#include "module/ecs/components/renderable.h"
#include "module/ecs/components/transform.h"
#include "module/ecs/components/world_transform.h"
#include "scene/scene.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    void RenderScene::syncFromScene(const std::shared_ptr<Scene>& scene)
    {
        if (!scene)
        {
            m_render_objects.clear();
            m_render_model_matrices.clear();
            m_lights.clear();
            m_render_entities.clear();
            m_light_entities.clear();
            m_cached_scene.reset();
            m_cached_generation = 0;
            return;
        }

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

    namespace
    {
        glm::mat4 getModelMatrix(Scene& scene, const entt::entity entity)
        {
            if (const auto* wt = scene.tryGet<WorldTransform>(entity))
                return wt->matrix;
            if (const auto* t = scene.tryGet<Transform>(entity))
                return t->getModelMatrix();
            return glm::mat4(1.0f);
        }

        glm::vec3 getWorldPosition(Scene& scene, const entt::entity entity)
        {
            if (auto* wt = scene.tryGet<WorldTransform>(entity))
                return glm::vec3(wt->matrix[3]);
            if (const auto* t = scene.tryGet<Transform>(entity))
                return t->position;
            return glm::vec3(0.0f);
        }

        glm::vec3 getWorldForward(Scene& scene, const entt::entity entity)
        {
            if (auto* wt = scene.tryGet<WorldTransform>(entity))
                return -glm::vec3(wt->matrix[0][2], wt->matrix[1][2], wt->matrix[2][2]);
            if (const auto* t = scene.tryGet<Transform>(entity))
                return t->getForward();
            return glm::vec3(0.0f, 0.0f, -1.0f);
        }
    } // namespace

    void RenderScene::fullSync(Scene& scene)
    {
        m_render_objects.clear();
        m_render_model_matrices.clear();
        m_lights.clear();
        m_render_entities.clear();
        m_light_entities.clear();

        const auto root = scene.getRoot();
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
                node->forEachChild([this, &scene](const std::shared_ptr<SceneNode>& child) { syncNode(scene, child); });
                return;
            }

            if (auto* r = scene.tryGet<Renderable>(entity))
            {
                if (r->render_object)
                {
                    m_render_objects.push_back(r->render_object);
                    m_render_model_matrices.push_back(getModelMatrix(scene, entity));
                    m_render_entities.push_back(entity);
                }
            }

            if (auto* pl = scene.tryGet<PointLight>(entity))
            {
                if (pl->enabled)
                {
                    Light light {};
                    light.type             = LightType::Point;
                    light.position         = getWorldPosition(scene, entity);
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
                    light.position         = getWorldPosition(scene, entity);
                    light.direction        = getWorldForward(scene, entity);
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
                    light.position         = getWorldPosition(scene, entity);
                    light.direction        = getWorldForward(scene, entity);
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
                    light.direction        = getWorldForward(scene, entity);
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

        node->forEachChild([this, &scene](const std::shared_ptr<SceneNode>& child) { syncNode(scene, child); });
    }

    int RenderScene::getDrawCallCount() const
    {
        int count = 0;
        for (const auto& ro : m_render_objects)
        {
            if (ro)
                count += static_cast<int>(ro->getMeshCount());
        }
        return count;
    }

    int RenderScene::getTriangleCount() const
    {
        int count = 0;
        for (const auto& ro : m_render_objects)
        {
            if (ro)
            {
                for (size_t i = 0; i < ro->getMeshCount(); ++i)
                    count += ro->getTriangleCount(i);
            }
        }
        return count;
    }

    void RenderScene::updateTransformsOnly(Scene& scene)
    {
        for (size_t i = 0; i < m_render_entities.size(); ++i)
        {
            const entt::entity entity = m_render_entities[i];
            if (!scene.valid(entity) || i >= m_render_objects.size() || i >= m_render_model_matrices.size())
                continue;

            m_render_model_matrices[i] = getModelMatrix(scene, entity);
        }

        for (size_t i = 0; i < m_light_entities.size(); ++i)
        {
            const entt::entity entity = m_light_entities[i];
            if (!scene.valid(entity) || i >= m_lights.size())
                continue;

            Light& light = m_lights[i];

            if (const auto* pl = scene.tryGet<PointLight>(entity))
            {
                light.position  = getWorldPosition(scene, entity);
                light.color     = pl->color;
                light.intensity = pl->enabled ? pl->intensity : 0.0f;
                light.constant  = pl->constant;
                light.linear    = pl->linear;
                light.quadratic = pl->quadratic;
                light.range     = pl->range;
            }
            else if (const auto* sl = scene.tryGet<SpotLight>(entity))
            {
                light.position         = getWorldPosition(scene, entity);
                light.direction        = getWorldForward(scene, entity);
                light.color            = sl->color;
                light.intensity        = sl->enabled ? sl->intensity : 0.0f;
                light.constant         = sl->constant;
                light.linear           = sl->linear;
                light.quadratic        = sl->quadratic;
                light.range            = sl->range;
                light.inner_cone_angle = sl->inner_cone_angle;
                light.outer_cone_angle = sl->outer_cone_angle;
            }
            else if (const auto* al = scene.tryGet<AreaLight>(entity))
            {
                light.position  = getWorldPosition(scene, entity);
                light.direction = getWorldForward(scene, entity);
                light.color     = al->color;
                light.intensity = al->enabled ? al->intensity : 0.0f;
                light.width     = al->width;
                light.height    = al->height;
            }
            else if (const auto* dl = scene.tryGet<DirectionalLight>(entity))
            {
                light.direction = getWorldForward(scene, entity);
                light.color     = dl->color;
                light.intensity = dl->enabled ? dl->intensity : 0.0f;
            }
        }
    }

    std::optional<std::reference_wrapper<const Light>> RenderScene::findDirectionalLight() const
    {
        for (const Light& light : m_lights)
        {
            if (light.type == LightType::Directional)
                return std::cref(light);
        }
        return std::nullopt;
    }

} // namespace RealmEngine
