#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "render/render_light.h"
#include "render/rhi.h"
#include <cstdint>

namespace RealmEngine
{
    constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 4;
    constexpr uint32_t MAX_POINT_LIGHTS       = 32;
    constexpr uint32_t MAX_SPOT_LIGHTS        = 16;

    struct CameraRes
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 view_projection;
        glm::vec3 camera_position;
        float     layout_pad0;
    };

    struct ObjectRes
    {
        glm::mat4 model;
        glm::mat4 normal_matrix;
    };

    struct LightingRes
    {
        glm::vec4 ambient_color;

        int32_t dir_light_count;
        int32_t point_light_count;
        int32_t spot_light_count;
        float   layout_pad0;

        DirectionalLight dir_lights[MAX_DIRECTIONAL_LIGHTS];
        PointLight       point_lights[MAX_POINT_LIGHTS];
        SpotLight        spot_lights[MAX_SPOT_LIGHTS];
    };

    class RenderResource
    {
    public:
        RenderResource()           = default;
        ~RenderResource() noexcept = default;

        RenderResource(const RenderResource&)            = delete;
        RenderResource& operator=(const RenderResource&) = delete;
        RenderResource(RenderResource&&)                 = delete;
        RenderResource& operator=(RenderResource&&)      = delete;

        void initialize(RHI& rhi);
        void dispose(RHI& rhi);

        void updateCameraBuf(RHI& rhi, const CameraRes& data);
        void updateObjectBuf(RHI& rhi, const ObjectRes& data);
        void updateLightingBuf(RHI& rhi, const LightingRes& data);

        void bindCameraBuf(uint32_t binding_point = 0);
        void bindObjectBuf(uint32_t binding_point = 1);
        void bindLightingBuf(uint32_t binding_point = 2);

        BufferHandle getCameraBuf() const;
        BufferHandle getObjectBuf() const;
        BufferHandle getLightingBuf() const;

    private:
        BufferHandle m_camera_buf {0};
        BufferHandle m_object_buf {0};
        BufferHandle m_lighting_buf {0};
    };

} // namespace RealmEngine