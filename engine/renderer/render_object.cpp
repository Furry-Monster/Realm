#include "renderer/render_object.h"

#include "rhi/rhi_device.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    RenderObject::RenderObject(std::vector<RenderMesh> meshes) : m_meshes(std::move(meshes)) {}

    int RenderObject::getTriangleCount(size_t mesh_index) const
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

    bool RenderObject::isHairMesh(const RenderMesh& mesh)
    {
        return mesh.m_material.properties.getBool("isHair");
    }

    bool RenderObject::isStandardOpaque(const RenderMesh& mesh)
    {
        return !isHairMesh(mesh) && !mesh.m_material.hasCustomShader() && mesh.m_material.isOpaque();
    }

    bool RenderObject::isStandardTransparent(const RenderMesh& mesh)
    {
        return !isHairMesh(mesh) && !mesh.m_material.hasCustomShader() && mesh.m_material.isTransparent();
    }

    bool RenderObject::hasTransparentMeshes() const
    {
        for (const auto& mesh : m_meshes)
        {
            if (isStandardTransparent(mesh))
                return true;
        }
        return false;
    }

    bool RenderObject::hasCustomShaderMeshes() const
    {
        for (const auto& mesh : m_meshes)
        {
            if (mesh.m_material.hasCustomShader())
                return true;
        }
        return false;
    }

    void RenderObject::drawOpaque(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
        {
            if (isStandardOpaque(mesh))
                mesh.draw(shader);
        }
    }

    void RenderObject::drawTransparent(RHIShader& shader, RHIDevice& device)
    {
        for (auto& mesh : m_meshes)
        {
            if (isStandardTransparent(mesh))
            {
                device.setCullFace(mesh.m_material.isDoubleSided() ? CullFace::None : CullFace::Back);
                mesh.draw(shader);
            }
        }
    }

    void RenderObject::drawHair(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
        {
            if (isHairMesh(mesh))
                mesh.draw(shader);
        }
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
            if (mesh.m_material.hasCustomShader() && !isHairMesh(mesh) && mesh.m_material.isOpaque())
                fn(mesh);
        }
    }

    void RenderObject::forEachCustomTransparentMesh(const std::function<void(RenderMesh&)>& fn)
    {
        for (auto& mesh : m_meshes)
        {
            if (mesh.m_material.hasCustomShader() && !isHairMesh(mesh) && mesh.m_material.isTransparent())
                fn(mesh);
        }
    }

    RenderMesh* RenderObject::getMesh(size_t index) { return index < m_meshes.size() ? &m_meshes[index] : nullptr; }

    const RenderMesh* RenderObject::getMesh(size_t index) const
    {
        return index < m_meshes.size() ? &m_meshes[index] : nullptr;
    }

} // namespace RealmEngine
