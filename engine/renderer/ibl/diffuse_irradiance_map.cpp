#include "renderer/ibl/diffuse_irradiance_map.h"

#include "core/geometry/primitive_vertices.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/ibl/ibl_geometry.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_types.h"
#include "rhi/rhi_vertex_input.h"

namespace RealmEngine
{
    DiffuseIrradianceMap::DiffuseIrradianceMap(RHIDevice&         device,
                                               const std::string& engine_root,
                                               RHITexture*        environment_cubemap) :
        m_environment_cubemap(environment_cubemap)
    {
        const std::string vert_path = engine_root + "/shaders/builtin/ibl/diffuseirradiance.vert";
        const std::string frag_path = engine_root + "/shaders/builtin/ibl/diffuseirradiance.frag";

        m_shader = device.createShader(vert_path, frag_path);
        if (!m_shader || !m_shader->isValid())
            return;

        FramebufferDesc desc;
        desc.width  = WIDTH;
        desc.height = HEIGHT;
        FramebufferAttachment color;
        color.format                          = TextureFormat::RGB16F;
        color.is_cubemap                      = true;
        desc.color_attachments                = {color};
        desc.has_depth                        = true;
        desc.depth_attachment.format          = TextureFormat::Depth24;
        desc.depth_attachment.is_renderbuffer = true;

        m_framebuffer = device.createFramebuffer(desc);
    }

    void DiffuseIrradianceMap::compute(RHIDevice& device)
    {
        if (!m_shader || !m_shader->isValid() || !m_framebuffer || !m_environment_cubemap)
            return;

        constexpr glm::mat4 model = glm::mat4(1.0f);
        constexpr glm::vec3 origin(0.0f, 0.0f, 0.0f);
        constexpr glm::vec3 unit_x(1.0f, 0.0f, 0.0f);
        constexpr glm::vec3 unit_y(0.0f, 1.0f, 0.0f);
        constexpr glm::vec3 unit_z(0.0f, 0.0f, 1.0f);

        const glm::mat4 camera_angles[] = {glm::lookAt(origin, unit_x, -unit_y),
                                           glm::lookAt(origin, -unit_x, -unit_y),
                                           glm::lookAt(origin, unit_y, unit_z),
                                           glm::lookAt(origin, -unit_y, -unit_z),
                                           glm::lookAt(origin, unit_z, -unit_y),
                                           glm::lookAt(origin, -unit_z, -unit_y)};
        const glm::mat4 projection      = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 2.0f);

        const auto cube_mesh = createIblCubeMesh(device);

        device.setViewport(0, 0, WIDTH, HEIGHT);
        m_framebuffer->bind();
        m_shader->use();
        m_shader->setInt("environmentCubemap", 0);
        device.bindTexture(0, *m_environment_cubemap);

        for (int i = 0; i < 6; i++)
        {
            m_shader->setMVP(model, camera_angles[i], projection);
            m_framebuffer->setCubeFace(i);
            device.clear(ClearFlags::Color | ClearFlags::Depth);
            cube_mesh.vertex_input->draw(PrimitiveType::Triangles,
                                         static_cast<uint32_t>(PrimitiveVertices::k_cube_vertex_count));
        }

        device.bindDefaultFramebuffer();
    }

    RHITexture* DiffuseIrradianceMap::getCubemapTexture() const
    {
        return m_framebuffer ? m_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
