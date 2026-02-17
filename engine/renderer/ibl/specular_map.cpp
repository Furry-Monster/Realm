#include "renderer/ibl/specular_map.h"

#include "core/geometry/primitive_vertices.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/ibl/ibl_geometry.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"
#include "rhi/rhi_vertex_input.h"

namespace RealmEngine
{
    SpecularMap::SpecularMap(RHIDevice& device, const std::string& engine_root, RHITexture* environment_cubemap) :
        m_environment_cubemap(environment_cubemap)
    {
        std::string prefilter_vert = engine_root + "/shaders/builtin/ibl/specularenv.vert";
        std::string prefilter_frag = engine_root + "/shaders/builtin/ibl/specularenv.frag";
        m_prefilter_shader         = device.createShader(prefilter_vert, prefilter_frag);

        FramebufferDesc prefilter_desc;
        prefilter_desc.width  = PREFILTER_WIDTH;
        prefilter_desc.height = PREFILTER_HEIGHT;
        FramebufferAttachment prefilter_color;
        prefilter_color.format                          = TextureFormat::RGB16F;
        prefilter_color.is_cubemap                      = true;
        prefilter_color.min_filter                      = TextureFilter::LinearMipmapLinear;
        prefilter_color.gen_mips                        = true;
        prefilter_desc.color_attachments                = {prefilter_color};
        prefilter_desc.has_depth                        = true;
        prefilter_desc.depth_attachment.format          = TextureFormat::Depth24;
        prefilter_desc.depth_attachment.is_renderbuffer = true;
        m_prefilter_framebuffer                         = device.createFramebuffer(prefilter_desc);

        std::string brdf_vert = engine_root + "/shaders/builtin/ibl/brdfconvolution.vert";
        std::string brdf_frag = engine_root + "/shaders/builtin/ibl/brdfconvolution.frag";
        m_brdf_shader         = device.createShader(brdf_vert, brdf_frag);

        FramebufferDesc brdf_desc;
        brdf_desc.width  = BRDF_WIDTH;
        brdf_desc.height = BRDF_HEIGHT;
        FramebufferAttachment brdf_color;
        brdf_color.format                          = TextureFormat::RG16F;
        brdf_desc.color_attachments                = {brdf_color};
        brdf_desc.has_depth                        = true;
        brdf_desc.depth_attachment.format          = TextureFormat::Depth24;
        brdf_desc.depth_attachment.is_renderbuffer = true;
        m_brdf_framebuffer                         = device.createFramebuffer(brdf_desc);
    }

    void SpecularMap::computePrefilteredEnvMap(RHIDevice& device)
    {
        if (!m_prefilter_shader || !m_prefilter_shader->isValid() || !m_prefilter_framebuffer || !m_environment_cubemap)
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

        auto cube_mesh = createIblCubeMesh(device);

        m_prefilter_framebuffer->bind();
        m_prefilter_shader->use();
        m_prefilter_shader->setInt("environmentCubemap", 0);
        device.bindTexture(0, *m_environment_cubemap);

        for (unsigned int mip = 0; mip < PREFILTER_MIP_LEVELS; mip++)
        {
            m_prefilter_framebuffer->setMipLevel(static_cast<int>(mip));
            int w = std::max(1, PREFILTER_WIDTH >> mip);
            int h = std::max(1, PREFILTER_HEIGHT >> mip);
            device.setViewport(0, 0, w, h);

            float roughness = static_cast<float>(mip) / static_cast<float>(PREFILTER_MIP_LEVELS - 1);
            m_prefilter_shader->setFloat("roughness", roughness);

            for (int i = 0; i < 6; i++)
            {
                m_prefilter_shader->setMVP(model, camera_angles[i], projection);
                m_prefilter_framebuffer->setCubeFace(i);
                device.clear(ClearFlags::Color | ClearFlags::Depth);
                cube_mesh.vertex_input->draw(PrimitiveType::Triangles,
                                             static_cast<uint32_t>(PrimitiveVertices::k_cube_vertex_count));
            }
        }

        device.bindDefaultFramebuffer();
    }

    void SpecularMap::computeBrdfConvolutionMap(RHIDevice& device)
    {
        if (!m_brdf_shader || !m_brdf_shader->isValid() || !m_brdf_framebuffer)
            return;

        auto quad_mesh = createIblFullscreenQuadMesh(device);

        m_brdf_framebuffer->bind();
        m_brdf_shader->use();
        device.setViewport(0, 0, BRDF_WIDTH, BRDF_HEIGHT);
        device.clear(ClearFlags::Color | ClearFlags::Depth);
        quad_mesh.vertex_input->draw(PrimitiveType::Triangles,
                                     static_cast<uint32_t>(PrimitiveVertices::k_fullscreen_quad_vertex_count));

        device.bindDefaultFramebuffer();
    }

    RHITexture* SpecularMap::getPrefilteredEnvMapTexture() const
    {
        return m_prefilter_framebuffer ? m_prefilter_framebuffer->getColorAttachment(0) : nullptr;
    }

    RHITexture* SpecularMap::getBrdfConvolutionTexture() const
    {
        return m_brdf_framebuffer ? m_brdf_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
