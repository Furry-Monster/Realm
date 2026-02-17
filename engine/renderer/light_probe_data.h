#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace RealmEngine
{
    class Scene;

    struct LightProbeGPUData
    {
        static constexpr int MAX_ACTIVE_PROBES = 64;

        struct alignas(16) ProbeInfo
        {
            glm::vec4 position_radius; // xyz = position, w = influence_radius
            glm::vec4 sh[9];           // rgb = SH coefficient, a = padding
        };

        int                    probe_count {0};
        std::vector<ProbeInfo> probes;

        void collectFromScene(Scene& scene);
    };

} // namespace RealmEngine
