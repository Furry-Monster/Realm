#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "math.h"
#include <cstdint>
namespace RealmEngine
{
    struct Frustum
    {
        // Order:left,right,bottom,top,near,far
        // vec4(A,B,C,D) ---> Ax + By + Cz + D = 0
        // vec3(A,B,C) ---> normal , float D ---> distance to origin(0,0,0).
        glm::vec4 planes[6];

        constexpr bool containsPoint(const glm::vec3& point) const;
        constexpr bool containsSphere(const glm::vec3& center, float r) const;
        constexpr bool containsAABB(const AABB& bounds) const;
    };

    enum class ProjectionType : uint8_t
    {
        Perspective,
        Orthographic
    };

    class RenderCamera
    {
    public:
        RenderCamera()           = default;
        ~RenderCamera() noexcept = default;

        RenderCamera(const RenderCamera&)                = delete;
        RenderCamera& operator=(const RenderCamera&)     = delete;
        RenderCamera(RenderCamera&&) noexcept            = default;
        RenderCamera& operator=(RenderCamera&&) noexcept = default;

        void setPosition(const glm::vec3& pos);
        void setRotation(const glm::quat& rotat);
        void setRotation(const glm::vec3& eular_angle);
        void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));

        const glm::vec3& getPosition() const;
        const glm::quat& getRotation() const;
        glm::vec3        getLocalForward() const;
        glm::vec3        getLocalRight() const;
        glm::vec3        getLocalUp() const;

        void setPerspective(float fov, float aspect_ratio, float near_plane, float far_plane);
        void serOrthographic(float left, float right, float bottom, float top, float near_plane, float far_plane);

        void  setFov(float fov);
        void  setAspectRatio(float aspect_ratio);
        void  setNearPlane(float near_plane);
        void  setFarPlane(float far_plane);
        float getFov() const;
        float getAspectRatio() const;
        float getNearPlane() const;
        float getFarPlane() const;

        ProjectionType getProjectionType() const;

        const glm::mat4& getViewMatrix() const;
        const glm::mat4& getProjMatrix() const;
        const glm::mat4& getViewProjMatrix() const;

        const Frustum& getFrustum() const;

        void update();

    protected:
        void updateViewMatrix();
        void updateProjectionMatrix();
        void extractFrustum();

        glm::vec3 m_position {0.0f, 0.0f, 0.0f};
        glm::quat m_rotation {1.0f, 0.0f, 0.0f, 0.0f};

        ProjectionType m_projection_type {ProjectionType::Perspective};
        float          m_fov {45.0f};
        float          m_aspect_ratio {16.0f / 9.0f};
        float          m_near_plane {0.1f};
        float          m_far_plane {1000.0f};

        float m_ortho_left {-10.0f};
        float m_ortho_right {10.0f};
        float m_ortho_bottom {-10.0f};
        float m_ortho_top {10.0f};

        glm::mat4 m_view_matrix {1.0f};
        glm::mat4 m_proj_matrix {1.0f};
        glm::mat4 m_view_proj_matrix {1.0f};

        Frustum m_frustum;

        bool m_view_mat_dirty {true};
        bool m_proj_mat_dirty {true};
    };

} // namespace RealmEngine