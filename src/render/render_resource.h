#pragma once

#include "render/render_material.h"
#include "render/render_mesh.h"
#include <cstdint>
#include <vector>

namespace RealmEngine
{
    using RenderResHandle      = uint32_t;
    using RenderMeshHandle     = RenderResHandle;
    using RenderMaterialHandle = RenderResHandle;

    class RenderResource
    {
    public:
        RenderResource()           = default;
        ~RenderResource() noexcept = default;

        RenderResource(const RenderResource&)            = delete;
        RenderResource& operator=(const RenderResource&) = delete;
        RenderResource(RenderResource&&)                 = default;
        RenderResource& operator=(RenderResource&&)      = default;

        void initialize();
        void disposal();

    private:
        std::vector<RenderMesh>     m_render_meshes;
        std::vector<RenderMaterial> m_render_materials;
    };

} // namespace RealmEngine