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

        m_render_objects.clear();
        m_lights.clear();

        auto root = scene->getRoot();
        if (root)
            syncNode(scene, root);
    }

    void RenderScene::syncNode(std::shared_ptr<Scene> scene, std::shared_ptr<SceneNode> node)
    {
        if (!node || !scene)
            return;

        if (node->hasEntity())
        {
            size_t entity_id = node->getEntityId();
            auto   entity    = scene->getEntity(entity_id);

            if (entity)
            {
                // Every entity must have a Transform component
                auto transform = entity->getComponent<Transform>();
                if (!transform)
                    transform = std::make_shared<Transform>();

                // Renderable
                auto renderable = entity->getComponent<Renderable>();
                if (renderable && renderable->hasRenderObject())
                {
                    auto render_obj = renderable->getRenderObject();
                    if (render_obj)
                    {
                        render_obj->setPosition(transform->getPosition());
                        render_obj->setOrientation(transform->getRotation());
                        render_obj->setScale(transform->getScale());

                        m_render_objects.push_back(render_obj);
                    }
                }

                // Point Light
                auto point_light = entity->getComponent<Point>();
                if (point_light && point_light->isEnabled())
                {
                    Light light {};
                    light.type             = LightType::Point;
                    light.position         = transform->getPosition();
                    light.direction        = glm::vec3(0.0f);
                    light.color            = point_light->getColor();
                    light.intensity        = point_light->getIntensity();
                    light.constant         = point_light->getConstantAttenuation();
                    light.linear           = point_light->getLinearAttenuation();
                    light.quadratic        = point_light->getQuadraticAttenuation();
                    light.range            = point_light->getRange();
                    light.inner_cone_angle = 0.0f;
                    light.outer_cone_angle = 0.0f;
                    light.width            = 0.0f;
                    light.height           = 0.0f;
                    m_lights.push_back(light);
                }

                // Spot Light
                auto spot_light = entity->getComponent<Spot>();
                if (spot_light && spot_light->isEnabled())
                {
                    Light light {};
                    light.type             = LightType::Spot;
                    light.position         = transform->getPosition();
                    light.direction        = -transform->getForward();
                    light.color            = spot_light->getColor();
                    light.intensity        = spot_light->getIntensity();
                    light.constant         = spot_light->getConstantAttenuation();
                    light.linear           = spot_light->getLinearAttenuation();
                    light.quadratic        = spot_light->getQuadraticAttenuation();
                    light.range            = spot_light->getRange();
                    light.inner_cone_angle = spot_light->getInnerConeAngle();
                    light.outer_cone_angle = spot_light->getOuterConeAngle();
                    light.width            = 0.0f;
                    light.height           = 0.0f;
                    m_lights.push_back(light);
                }

                // Area Light
                auto area_light = entity->getComponent<Area>();
                if (area_light && area_light->isEnabled())
                {
                    Light light {};
                    light.type             = LightType::Area;
                    light.position         = transform->getPosition();
                    light.direction        = -transform->getForward();
                    light.color            = area_light->getColor();
                    light.intensity        = area_light->getIntensity();
                    light.constant         = 0.0f;
                    light.linear           = 0.0f;
                    light.quadratic        = 0.0f;
                    light.range            = 0.0f;
                    light.inner_cone_angle = 0.0f;
                    light.outer_cone_angle = 0.0f;
                    light.width            = area_light->getWidth();
                    light.height           = area_light->getHeight();
                    m_lights.push_back(light);
                }

                // Directional Light
                auto directional_light = entity->getComponent<Directional>();
                if (directional_light && directional_light->isEnabled())
                {
                    Light light {};
                    light.type             = LightType::Directional;
                    light.position         = glm::vec3(0.0f);
                    light.direction        = -transform->getForward();
                    light.color            = directional_light->getColor();
                    light.intensity        = directional_light->getIntensity();
                    light.constant         = 0.0f;
                    light.linear           = 0.0f;
                    light.quadratic        = 0.0f;
                    light.range            = 0.0f;
                    light.inner_cone_angle = 0.0f;
                    light.outer_cone_angle = 0.0f;
                    light.width            = 0.0f;
                    light.height           = 0.0f;
                    m_lights.push_back(light);
                }
            }
        }

        node->forEachChild([this, scene](std::shared_ptr<SceneNode> child) { syncNode(scene, child); });
    }

} // namespace RealmEngine
