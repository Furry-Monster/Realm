#include "renderer/render_object.h"

#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    RenderObject::RenderObject(std::vector<RenderMesh> meshes) : m_meshes(std::move(meshes)) {}

    int RenderObject::getTriangleCount(const size_t mesh_index) const
    {
        if (mesh_index >= m_meshes.size())
            return 0;
        return m_meshes[mesh_index].getTriangleCount();
    }

    void RenderObject::forEachMesh(const std::function<void(RenderMesh&)>& fn)
    {
        for (auto& mesh : m_meshes)
            fn(mesh);
    }

    void RenderObject::drawShadow(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.drawShadow(shader);
    }

    RenderMesh* RenderObject::getMesh(const size_t index)
    {
        return index < m_meshes.size() ? &m_meshes[index] : nullptr;
    }

    const RenderMesh* RenderObject::getMesh(const size_t index) const
    {
        return index < m_meshes.size() ? &m_meshes[index] : nullptr;
    }

} // namespace RealmEngine
