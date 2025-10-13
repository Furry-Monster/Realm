#include "render_resource.h"
#include <glad/gl.h>

namespace RealmEngine
{
    void RenderResource::initialize(RHI& rhi)
    {
        m_camera_buf   = rhi.createUniformBuffer(sizeof(CameraRes), nullptr, GL_DYNAMIC_DRAW);
        m_object_buf   = rhi.createUniformBuffer(sizeof(ObjectRes), nullptr, GL_DYNAMIC_DRAW);
        m_lighting_buf = rhi.createUniformBuffer(sizeof(LightingRes), nullptr, GL_DYNAMIC_DRAW);
    }

    void RenderResource::dispose(RHI& rhi)
    {
        if (m_camera_buf != 0)
            rhi.deleteBuffer(m_camera_buf);
        if (m_object_buf != 0)
            rhi.deleteBuffer(m_object_buf);
        if (m_lighting_buf != 0)
            rhi.deleteBuffer(m_lighting_buf);

        m_camera_buf   = 0;
        m_object_buf   = 0;
        m_lighting_buf = 0;
    }

    void RenderResource::updateCameraBuf(RHI& rhi, const CameraRes& data)
    {
        rhi.updateBuffer(m_camera_buf, GL_UNIFORM_BUFFER, 0, sizeof(CameraRes), &data);
    }

    void RenderResource::updateObjectBuf(RHI& rhi, const ObjectRes& data)
    {
        rhi.updateBuffer(m_object_buf, GL_UNIFORM_BUFFER, 0, sizeof(ObjectRes), &data);
    }

    void RenderResource::updateLightingBuf(RHI& rhi, const LightingRes& data)
    {
        rhi.updateBuffer(m_lighting_buf, GL_UNIFORM_BUFFER, 0, sizeof(LightingRes), &data);
    }

    void RenderResource::bindCameraBuf(uint32_t binding_point)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_camera_buf);
    }

    void RenderResource::bindObjectBuf(uint32_t binding_point)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_object_buf);
    }

    void RenderResource::bindLightingBuf(uint32_t binding_point)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_lighting_buf);
    }

    BufferHandle RenderResource::getCameraBuf() const { return m_camera_buf; }

    BufferHandle RenderResource::getObjectBuf() const { return m_object_buf; }

    BufferHandle RenderResource::getLightingBuf() const { return m_lighting_buf; }

} // namespace RealmEngine