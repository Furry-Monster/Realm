#pragma once

#include "render/render_camera.h"
#include "render/render_material.h"
#include "render/render_mesh.h"
#include "render/render_scene.h"
#include <memory>
#include <vector>

namespace RealmEngine
{
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

        void update(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera);

    private:
        std::vector<RenderMesh>     m_render_meshes;
        std::vector<RenderMaterial> m_render_materials;
    };

} // namespace RealmEngine