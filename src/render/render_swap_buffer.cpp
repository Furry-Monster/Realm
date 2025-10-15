#include "render_swap_buffer.h"
#include "utils.h"
#include <glad/gl.h>
#include <utility>

namespace RealmEngine
{
    void       ObjectsQueue::add(ObjectRes& res) { objects.push_back(res); }
    void       ObjectsQueue::pop() { objects.pop_front(); }
    bool       ObjectsQueue::isEmpty() const { return objects.empty(); }
    ObjectRes& ObjectsQueue::getNextObject() { return objects.front(); }

    void RenderSwapData::addDirtyObject(ObjectRes&& res)
    {
        if (dirty_objects.has_value())
        {
            dirty_objects->add(res);
        }
        else
        {
            ObjectsQueue objects;
            objects.add(res);
            dirty_objects = std::move(objects);
        }
    }
    void RenderSwapData::addRemovedObject(ObjectRes&& res)
    {
        if (removed_objects.has_value())
        {
            removed_objects->add(res);
        }
        else
        {
            ObjectsQueue objects;
            objects.add(res);
            removed_objects = std::move(objects);
        }
    }

    void RenderSwapBuffer::initialize() { info("Render Swap Buffer initialized."); }

    void RenderSwapBuffer::dispose() { info("Render Swap Buffer disposed all resources."); }

    RenderSwapData&       RenderSwapBuffer::getLogicData() { return m_logic_data; }
    const RenderSwapData& RenderSwapBuffer::getLogicData() const { return m_logic_data; }
    RenderSwapData&       RenderSwapBuffer::getRenderData() { return m_render_data; }
    const RenderSwapData& RenderSwapBuffer::getRenderData() const { return m_render_data; }

    void RenderSwapBuffer::swapData()
    {
        if (!isReadyToSwap())
            return;

        resetDirtyCamera();
        resetDirtyLighting();
        resetDirtyObjects();
        resetRemovedObjects();

        std::swap(m_logic_data, m_render_data);
    }

    constexpr bool RenderSwapBuffer::isReadyToSwap() const
    {
        return m_render_data.dirty_camera.has_value() || m_render_data.dirty_lighting.has_value() ||
               m_render_data.dirty_objects.has_value() || m_render_data.removed_objects.has_value();
    }

    void RenderSwapBuffer::resetDirtyCamera() { m_render_data.dirty_camera.reset(); }
    void RenderSwapBuffer::resetDirtyLighting() { m_render_data.dirty_lighting.reset(); }
    void RenderSwapBuffer::resetDirtyObjects() { m_render_data.dirty_objects.reset(); }
    void RenderSwapBuffer::resetRemovedObjects() { m_render_data.removed_objects.reset(); }

} // namespace RealmEngine