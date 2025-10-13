#include "render_camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "resource/asset_manager.h"

namespace RealmEngine
{

    void RenderCamera::setPosition(const glm::vec3& pos)
    {
        m_position       = pos;
        m_view_mat_dirty = true;
    }
    void RenderCamera::setRotation(const glm::quat& rotat)
    {
        m_rotation       = glm::normalize(rotat);
        m_view_mat_dirty = true;
    }
    void RenderCamera::setRotation(const glm::vec3& eular_angle)
    {
        glm::quat q_pitch = glm::angleAxis(glm::radians(eular_angle.x), glm::vec3(1, 0, 0));
        glm::quat q_yaw   = glm::angleAxis(glm::radians(eular_angle.y), glm::vec3(0, 1, 0));
        glm::quat q_roll  = glm::angleAxis(glm::radians(eular_angle.z), glm::vec3(0, 0, 1));

        m_rotation       = glm::normalize(q_yaw * q_pitch * q_roll);
        m_view_mat_dirty = true;
    }
    void RenderCamera::lookAt(const glm::vec3& target, const glm::vec3& up)
    {
        glm::vec3 local_forward = glm::normalize(target - m_position);
        glm::vec3 local_right   = glm::normalize(glm::cross(local_forward, up));
        glm::vec3 local_up      = glm::normalize(glm::cross(local_right, local_forward));

        glm::mat3 rotation_matrix;
        rotation_matrix[0] = local_right;
        rotation_matrix[1] = local_up;
        rotation_matrix[2] = -local_forward;

        m_rotation       = glm::quat_cast(rotation_matrix);
        m_view_mat_dirty = true;
    }

    const glm::vec3& RenderCamera::getPosition() const { return m_position; }
    const glm::quat& RenderCamera::getRotation() const { return m_rotation; }
    glm::vec3        RenderCamera::getLocalForward() const { return m_rotation * glm::vec3(0, 0, -1); }
    glm::vec3        RenderCamera::getLocalRight() const { return m_rotation * glm::vec3(1, 0, 0); }
    glm::vec3        RenderCamera::getLocalUp() const { return m_rotation * glm::vec3(0, 1, 0); }

    void RenderCamera::setPerspective(float fov, float aspect_ratio, float near_plane, float far_plane)
    {
        m_projection_type = ProjectionType::Perspective;
        m_fov             = fov;
        m_aspect_ratio    = aspect_ratio;
        m_near_plane      = near_plane;
        m_far_plane       = far_plane;
        m_proj_mat_dirty  = true;
    }
    void
    RenderCamera::serOrthographic(float left, float right, float bottom, float top, float near_plane, float far_plane)
    {
        m_projection_type = ProjectionType::Orthographic;
        m_ortho_left      = left;
        m_ortho_right     = right;
        m_ortho_bottom    = bottom;
        m_ortho_top       = top;
        m_near_plane      = near_plane;
        m_far_plane       = far_plane;
        m_proj_mat_dirty  = true;
    }

    void RenderCamera::setFov(float fov)
    {
        m_fov            = fov;
        m_proj_mat_dirty = true;
    }
    void RenderCamera::setAspectRatio(float aspect_ratio)
    {
        m_aspect_ratio   = aspect_ratio;
        m_proj_mat_dirty = true;
    }
    void RenderCamera::setNearPlane(float near_plane)
    {
        m_near_plane     = near_plane;
        m_proj_mat_dirty = true;
    }
    void RenderCamera::setFarPlane(float far_plane)
    {
        m_far_plane      = far_plane;
        m_proj_mat_dirty = true;
    }
    float RenderCamera::getFov() const { return m_fov; }
    float RenderCamera::getAspectRatio() const { return m_aspect_ratio; }
    float RenderCamera::getNearPlane() const { return m_near_plane; }
    float RenderCamera::getFarPlane() const { return m_far_plane; }

    ProjectionType RenderCamera::getProjectionType() const { return m_projection_type; }

    const glm::mat4& RenderCamera::getViewMatrix() const { return m_view_matrix; }
    const glm::mat4& RenderCamera::getProjMatrix() const { return m_proj_matrix; }
    const glm::mat4& RenderCamera::getViewProjMatrix() const { return m_view_proj_matrix; }

    const Frustum& RenderCamera::getFrustum() const { return m_frustum; }

    void RenderCamera::update()
    {
        if (!m_view_mat_dirty && !m_proj_mat_dirty)
            return;

        if (m_view_mat_dirty)
        {
            updateViewMatrix();
            m_view_matrix = false;
        }

        if (m_proj_mat_dirty)
        {
            updateProjectionMatrix();
            m_proj_mat_dirty = false;
        }

        m_view_proj_matrix = m_proj_matrix * m_view_matrix;
        extractFrustum();
    }

    void RenderCamera::updateViewMatrix()
    {
        glm::mat4 rotation_matrix    = glm::mat4_cast(m_rotation);
        glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), -m_position);
        m_view_matrix                = rotation_matrix * translation_matrix;
    }

    void RenderCamera::updateProjectionMatrix()
    {
        switch (m_projection_type)
        {
            case ProjectionType::Perspective:
                m_proj_matrix = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near_plane, m_far_plane);
                break;
            case ProjectionType::Orthographic:
                m_proj_matrix =
                    glm::ortho(m_ortho_left, m_ortho_right, m_ortho_bottom, m_ortho_top, m_near_plane, m_far_plane);
                break;
            default:
                err("Unknown projection type.");
                break;
        }
    }

    void RenderCamera::extractFrustum()
    {
        glm::mat4 vp = m_view_proj_matrix;

        m_frustum.planes[0] =
            glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]);
        m_frustum.planes[1] =
            glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]);
        m_frustum.planes[2] =
            glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]);
        m_frustum.planes[3] =
            glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]);
        m_frustum.planes[4] =
            glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]);
        m_frustum.planes[5] =
            glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]);

        for (auto& plane : m_frustum.planes)
        {
            float length = glm::length(glm::vec3(plane));
            plane /= length;
        }
    }

} // namespace RealmEngine