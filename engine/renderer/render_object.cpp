#include "renderer/render_object.h"

#include "rhi/rhi_device.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    RenderObject::RenderObject(std::vector<RenderMesh> meshes) : m_meshes(std::move(meshes)) {}

    bool RenderObject::hasTransparentMeshes() const
    {
        for (const auto& mesh : m_meshes)
        {
            if (!mesh.isHair() && !mesh.hasCustomShader() && mesh.isTransparent())
                return true;
        }
        return false;
    }

    bool RenderObject::hasCustomShaderMeshes() const
    {
        for (const auto& mesh : m_meshes)
        {
            if (mesh.hasCustomShader())
                return true;
        }
        return false;
    }

    int RenderObject::getTriangleCount(size_t mesh_index) const
    {
        if (mesh_index >= m_meshes.size())
            return 0;
        return m_meshes[mesh_index].getTriangleCount();
    }

    void RenderObject::draw(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.draw(shader);
    }

    void RenderObject::drawOpaque(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
        {
            if (!mesh.isHair() && !mesh.isTransparent() && !mesh.hasCustomShader())
                mesh.draw(shader);
        }
    }

    void RenderObject::drawTransparent(RHIShader& shader, RHIDevice& device)
    {
        for (auto& mesh : m_meshes)
        {
            if (!mesh.isHair() && mesh.isTransparent() && !mesh.hasCustomShader())
            {
                device.setCullFace(mesh.m_material.double_sided ? CullFace::None : CullFace::Back);
                mesh.draw(shader);
            }
        }
    }

    void RenderObject::drawHair(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.drawHair(shader);
    }

    RenderMesh* RenderObject::getMesh(size_t index) { return index < m_meshes.size() ? &m_meshes[index] : nullptr; }

    const RenderMesh* RenderObject::getMesh(size_t index) const
    {
        return index < m_meshes.size() ? &m_meshes[index] : nullptr;
    }

    void RenderObject::drawShadow(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.drawShadow(shader);
    }

    void RenderObject::forEachCustomOpaqueMesh(const std::function<void(RenderMesh&)>& fn)
    {
        for (auto& mesh : m_meshes)
        {
            if (mesh.hasCustomShader() && !mesh.isHair() && !mesh.isTransparent())
                fn(mesh);
        }
    }

    void RenderObject::forEachCustomTransparentMesh(const std::function<void(RenderMesh&)>& fn)
    {
        for (auto& mesh : m_meshes)
        {
            if (mesh.hasCustomShader() && !mesh.isHair() && mesh.isTransparent())
                fn(mesh);
        }
    }

} // namespace RealmEngine
