#include "render_swap_buffer.h"
#include "utils.h"
#include <glad/gl.h>

namespace RealmEngine
{
    void RenderSwapBuffer::initialize()
    {
        // m_camera_buf   = rhi.createUniformBuffer(sizeof(CameraRes), nullptr, GL_DYNAMIC_DRAW);
        // m_object_buf   = rhi.createUniformBuffer(sizeof(ObjectRes), nullptr, GL_DYNAMIC_DRAW);
        // m_lighting_buf = rhi.createUniformBuffer(sizeof(LightingRes), nullptr, GL_DYNAMIC_DRAW);

        info("Render Swap Buffer initialized.");
    }

    void RenderSwapBuffer::dispose()
    {
        // if (m_camera_buf != 0)
        //     rhi.deleteBuffer(m_camera_buf);
        // if (m_object_buf != 0)
        //     rhi.deleteBuffer(m_object_buf);
        // if (m_lighting_buf != 0)
        //     rhi.deleteBuffer(m_lighting_buf);

        m_camera_buf   = 0;
        m_object_buf   = 0;
        m_lighting_buf = 0;

        info("Render Swap Buffer disposed all resources.");
    }

    void RenderSwapBuffer::updateCameraBuf(RHI& rhi, const CameraRes& data) const
    {
        rhi.updateBuffer(m_camera_buf, GL_UNIFORM_BUFFER, 0, sizeof(CameraRes), &data);
    }

    void RenderSwapBuffer::updateObjectBuf(RHI& rhi, const ObjectRes& data) const
    {
        rhi.updateBuffer(m_object_buf, GL_UNIFORM_BUFFER, 0, sizeof(ObjectRes), &data);
    }

    void RenderSwapBuffer::updateLightingBuf(RHI& rhi, const LightingRes& data) const
    {
        rhi.updateBuffer(m_lighting_buf, GL_UNIFORM_BUFFER, 0, sizeof(LightingRes), &data);
    }

    void RenderSwapBuffer::bindCameraBuf(uint32_t binding_point) const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_camera_buf);
    }

    void RenderSwapBuffer::bindObjectBuf(uint32_t binding_point) const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_object_buf);
    }

    void RenderSwapBuffer::bindLightingBuf(uint32_t binding_point) const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_lighting_buf);
    }

    BufferHandle RenderSwapBuffer::getCameraBuf() const { return m_camera_buf; }

    BufferHandle RenderSwapBuffer::getObjectBuf() const { return m_object_buf; }

    BufferHandle RenderSwapBuffer::getLightingBuf() const { return m_lighting_buf; }

} // namespace RealmEngine