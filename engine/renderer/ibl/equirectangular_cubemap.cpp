#include "renderer/ibl/equirectangular_cubemap.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/ibl/hdri_cube.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    EquirectangularCubemap::EquirectangularCubemap(RHIDevice&         device,
                                                   const std::string& engine_root,
                                                   const std::string& hdri_path)
    {
        std::string vert_path = engine_root + "/shaders/builtin/ibl/hdricube.vert";
        std::string frag_path = engine_root + "/shaders/builtin/ibl/hdricube.frag";

        m_hdri_shader = device.createShader(vert_path, frag_path);
        if (!m_hdri_shader || !m_hdri_shader->isValid())
            return;

        m_hdri_cube = std::make_unique<HDRICube>(device, hdri_path);

        FramebufferDesc desc;
        desc.width  = m_cubemap_width;
        desc.height = m_cubemap_height;
        FramebufferAttachment color;
        color.format                          = TextureFormat::RGB16F;
        color.is_cubemap                      = true;
        color.gen_mips                        = true;
        desc.color_attachments                = {color};
        desc.has_depth                        = true;
        desc.depth_attachment.format          = TextureFormat::Depth24;
        desc.depth_attachment.is_renderbuffer = true;

        m_framebuffer = device.createFramebuffer(desc);
    }

    void EquirectangularCubemap::compute(RHIDevice& device)
    {
        if (!m_hdri_shader || !m_hdri_shader->isValid() || !m_framebuffer)
            return;

        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 origin(0.0f, 0.0f, 0.0f);
        glm::vec3 unit_x(1.0f, 0.0f, 0.0f);
        glm::vec3 unit_y(0.0f, 1.0f, 0.0f);
        glm::vec3 unit_z(0.0f, 0.0f, 1.0f);

        glm::mat4 camera_angles[] = {glm::lookAt(origin, unit_x, -unit_y),
                                     glm::lookAt(origin, -unit_x, -unit_y),
                                     glm::lookAt(origin, unit_y, unit_z),
                                     glm::lookAt(origin, -unit_y, -unit_z),
                                     glm::lookAt(origin, unit_z, -unit_y),
                                     glm::lookAt(origin, -unit_z, -unit_y)};
        glm::mat4 projection      = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 2.0f);

        device.setViewport(0, 0, m_cubemap_width, m_cubemap_height);
        m_framebuffer->bind();
        m_hdri_shader->use();

        for (int i = 0; i < 6; i++)
        {
            m_hdri_shader->setMVP(model, camera_angles[i], projection);
            m_framebuffer->setCubeFace(i);
            device.clear(ClearFlags::Color | ClearFlags::Depth);
            m_hdri_cube->draw(device, *m_hdri_shader);
        }

        RHITexture* color_tex = m_framebuffer->getColorAttachment(0);
        if (color_tex)
            color_tex->generateMipmaps();

        device.bindDefaultFramebuffer();
    }

    RHITexture* EquirectangularCubemap::getCubemapTexture() const
    {
        return m_framebuffer ? m_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
