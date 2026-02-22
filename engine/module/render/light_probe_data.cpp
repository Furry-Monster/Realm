#include "module/render/light_probe_data.h"

#include "module/ecs/components/lighting/light_probe.h"
#include "functional/ecs/components/transform.h"
#include "functional/ecs/components/world_transform.h"
#include "functional/scene/scene.h"

#include <cstddef>

namespace RealmEngine
{
    void LightProbeGPUData::collectFromScene(Scene& scene)
    {
        probes.clear();
        probe_count = 0;

        auto&      registry = scene.getRegistry();
        const auto view     = registry.view<LightProbe>();

        for (const auto entity : view)
        {
            if (probe_count >= MAX_ACTIVE_PROBES)
                break;

            auto& lp = view.get<LightProbe>(entity);
            if (!lp.enabled)
                continue;

            glm::vec3 position {0.0f};
            if (auto* wt = scene.tryGet<WorldTransform>(entity))
                position = glm::vec3(wt->matrix[3]);
            else if (const auto* t = scene.tryGet<Transform>(entity))
                position = t->position;

            ProbeInfo info {};
            info.position_radius = glm::vec4(position, lp.influence_radius);

            for (size_t i = 0; i < 9; ++i)
                info.sh[i] = glm::vec4(lp.sh_coefficients[i], 0.0f);

            probes.push_back(info);
            ++probe_count;
        }
    }

} // namespace RealmEngine
