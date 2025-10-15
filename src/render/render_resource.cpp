#include "render_resource.h"

#include "utils.h"

namespace RealmEngine
{
    void RenderResource::initialize() { info("Render Resource Manager initialized."); }
    void RenderResource::disposal() { info("Render Resource Manager dispoed all resource."); }

    void RenderResource::update(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) {}
    
    uint32_t RenderResource::addRenderMesh(RenderMesh&& mesh)
    {
        m_render_meshes.push_back(std::move(mesh));
        return static_cast<uint32_t>(m_render_meshes.size() - 1);
    }
    
    uint32_t RenderResource::addRenderMaterial(RenderMaterial&& material)
    {
        m_render_materials.push_back(std::move(material));
        return static_cast<uint32_t>(m_render_materials.size() - 1);
    }
    
    RenderMesh* RenderResource::getRenderMesh(uint32_t index)
    {
        if (index >= m_render_meshes.size())
            return nullptr;
        return &m_render_meshes[index];
    }
    
    RenderMaterial* RenderResource::getRenderMaterial(uint32_t index)
    {
        if (index >= m_render_materials.size())
            return nullptr;
        return &m_render_materials[index];
    }
} // namespace RealmEngine