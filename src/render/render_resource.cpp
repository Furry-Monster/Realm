#include "render_resource.h"
#include <glad/gl.h>

namespace RealmEngine
{
    void RenderResource::initialize(RHI& res_mgr)
    {
        m_camera_ubo   = res_mgr.createUniformBuffer(sizeof(CameraUBO), nullptr, GL_DYNAMIC_DRAW);
        m_object_ubo   = res_mgr.createUniformBuffer(sizeof(ObjectUBO), nullptr, GL_DYNAMIC_DRAW);
        m_lighting_ubo = res_mgr.createUniformBuffer(sizeof(LightingUBO), nullptr, GL_DYNAMIC_DRAW);
    }

    void RenderResource::dispose(RHI& res_mgr)
    {
        if (m_camera_ubo != 0)
            res_mgr.deleteBuffer(m_camera_ubo);
        if (m_object_ubo != 0)
            res_mgr.deleteBuffer(m_object_ubo);
        if (m_lighting_ubo != 0)
            res_mgr.deleteBuffer(m_lighting_ubo);

        m_camera_ubo   = 0;
        m_object_ubo   = 0;
        m_lighting_ubo = 0;
    }

    void RenderResource::updateCameraUBO(RHI& res_mgr, const CameraUBO& data)
    {
        res_mgr.updateBuffer(m_camera_ubo, GL_UNIFORM_BUFFER, 0, sizeof(CameraUBO), &data);
    }

    void RenderResource::updateObjectUBO(RHI& res_mgr, const ObjectUBO& data)
    {
        res_mgr.updateBuffer(m_object_ubo, GL_UNIFORM_BUFFER, 0, sizeof(ObjectUBO), &data);
    }

    void RenderResource::updateLightingUBO(RHI& res_mgr, const LightingUBO& data)
    {
        res_mgr.updateBuffer(m_lighting_ubo, GL_UNIFORM_BUFFER, 0, sizeof(LightingUBO), &data);
    }

    void RenderResource::bindCameraUBO(uint32_t binding_point)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_camera_ubo);
    }

    void RenderResource::bindObjectUBO(uint32_t binding_point)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_object_ubo);
    }

    void RenderResource::bindLightingUBO(uint32_t binding_point)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_lighting_ubo);
    }

} // namespace RealmEngine