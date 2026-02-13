#include "renderer/light.h"

#include <algorithm>
#include <cstring>

namespace RealmEngine
{
    LightUBO::LightUBO()
    {
        glGenBuffers(1, &m_ubo_id);
        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_id);
        glBufferData(GL_UNIFORM_BUFFER, BUFFER_SIZE, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    LightUBO::~LightUBO() noexcept
    {
        if (m_ubo_id != 0)
            glDeleteBuffers(1, &m_ubo_id);
    }

    LightUBO::LightUBO(LightUBO&& other) noexcept : m_ubo_id(other.m_ubo_id) { other.m_ubo_id = 0; }

    LightUBO& LightUBO::operator=(LightUBO&& other) noexcept
    {
        if (this != &other)
        {
            if (m_ubo_id != 0)
                glDeleteBuffers(1, &m_ubo_id);

            m_ubo_id       = other.m_ubo_id;
            other.m_ubo_id = 0;
        }

        return *this;
    }

    void LightUBO::updateLights(const std::vector<Light>& lights) const
    {
        size_t light_count = std::min(lights.size(), MAX_LIGHTS);

        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_id);

        int count = static_cast<int>(light_count);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &count);

        LightData light_data[MAX_LIGHTS];
        for (size_t i = 0; i < light_count; ++i)
        {
            const auto& light         = lights[i];
            light_data[i].position    = glm::vec4(light.position, static_cast<float>(static_cast<int>(light.type)));
            light_data[i].direction   = glm::vec4(light.direction, light.intensity);
            light_data[i].color       = glm::vec4(light.color, light.constant);
            light_data[i].attenuation = glm::vec4(light.linear, light.quadratic, light.range, light.inner_cone_angle);
            light_data[i].spot_area   = glm::vec4(light.outer_cone_angle, light.width, light.height, 0.0f);
        }

        // Zero-fill remaining slots
        for (size_t i = light_count; i < MAX_LIGHTS; ++i)
            light_data[i] = LightData {};

        glBufferSubData(GL_UNIFORM_BUFFER, 16, MAX_LIGHTS * sizeof(LightData), light_data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void LightUBO::bind(unsigned int binding_point) const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_ubo_id);
    }

} // namespace RealmEngine
