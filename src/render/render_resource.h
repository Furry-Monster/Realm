#pragma once

#include "render/render_material.h"
#include "render/render_mesh.h"
#include <vector>
namespace RealmEngine
{
    struct RenderResHandle
    {};

    class RenderResource
    {
    public:
        RenderResource()           = default;
        ~RenderResource() noexcept = default;

        RenderResource(const RenderResource&)            = delete;
        RenderResource& operator=(const RenderResource&) = delete;
        RenderResource(RenderResource&&)                 = default;
        RenderResource& operator=(RenderResource&&)      = default;

    private:
        std::vector<RenderMesh>     m_render_meshes;
        std::vector<RenderMaterial> m_render_materials;
    };

} // namespace RealmEngine