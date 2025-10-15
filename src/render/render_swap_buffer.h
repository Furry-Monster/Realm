#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "render/render_light.h"
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

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

    struct RenderObjectRes
    {
        glm::mat4 model;
        glm::mat4 normal_matrix;

        uint32_t mesh;
        uint32_t material;
    };

    struct ObjectRes
    {
        bool                         static_mesh {true};
        std::vector<RenderObjectRes> render_objects;
    };

    struct ObjectsQueue
    {
        std::deque<ObjectRes> objects;

        void add(ObjectRes& res);
        void pop();

        bool isEmpty() const;

        ObjectRes& getNextObject();
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

    struct RenderSwapData
    {
        std::optional<CameraRes>    dirty_camera;
        std::optional<LightingRes>  dirty_lighting;
        std::optional<ObjectsQueue> dirty_objects;
        std::optional<ObjectsQueue> removed_objects;

        void addDirtyObject(ObjectRes&& res);
        void addRemovedObject(ObjectRes&& res);
    };

    class RenderSwapBuffer
    {
    public:
        RenderSwapBuffer()           = default;
        ~RenderSwapBuffer() noexcept = default;

        RenderSwapBuffer(const RenderSwapBuffer&)            = delete;
        RenderSwapBuffer& operator=(const RenderSwapBuffer&) = delete;
        RenderSwapBuffer(RenderSwapBuffer&&)                 = delete;
        RenderSwapBuffer& operator=(RenderSwapBuffer&&)      = delete;

        void initialize();
        void dispose();

        RenderSwapData&       getLogicData();
        const RenderSwapData& getLogicData() const;
        RenderSwapData&       getRenderData();
        const RenderSwapData& getRenderData() const;

        constexpr bool isReadyToSwap() const;
        void           swapData();

        void resetDirtyCamera();
        void resetDirtyLighting();
        void resetDirtyObjects();
        void resetRemovedObjects();

    private:
        RenderSwapData m_logic_data;
        RenderSwapData m_render_data;
    };

} // namespace RealmEngine