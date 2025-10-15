#include "render_material.h"
#include <glad/gl.h>

namespace RealmEngine
{
    ProgramHandle RenderMaterial::getShaderProgram() const { return m_shader; }

    std::optional<TextureHandle> RenderMaterial::getBaseColorTexture() const { return m_base_color_tex; }
    std::optional<TextureHandle> RenderMaterial::getMetallicRoughnessTexture() const
    {
        return m_metallic_roughness_tex;
    }
    std::optional<TextureHandle> RenderMaterial::getNormalTexture() const { return m_normal_tex; }
    std::optional<TextureHandle> RenderMaterial::getOcclusionTexture() const { return m_occlusion_tex; }
    std::optional<TextureHandle> RenderMaterial::getEmissiveTexture() const { return m_emissive_tex; }

    const glm::vec4& RenderMaterial::getBaseColorFactor() const { return m_base_color_factor; }
    float            RenderMaterial::getMetallicFactor() const { return m_metallic_factor; }
    float            RenderMaterial::getRoughnessFactor() const { return m_roughness_factor; }
    const glm::vec3& RenderMaterial::getEmissiveFactor() const { return m_emissive_factor; }

    const RenderState& RenderMaterial::getRenderState() const { return m_render_state; }

    void RenderMaterial::sync(RHI& rhi, const Material& material, ProgramHandle shader_program)
    {
        m_shader = shader_program;
        
        m_base_color_factor = material.getBaseColorFactor();
        m_metallic_factor   = material.getMetallicFactor();
        m_roughness_factor  = material.getRoughnessFactor();
        m_emissive_factor   = material.getEmissiveFactor();

        if (const auto& base_color_tex = material.getBaseColorTexture())
        {
            TextureHandle handle = rhi.loadTexture(base_color_tex->path);
            m_base_color_tex     = handle;
        }

        if (const auto& metallic_roughness_tex = material.getMetallicRoughnessTexture())
        {
            TextureHandle handle     = rhi.loadTexture(metallic_roughness_tex->path);
            m_metallic_roughness_tex = handle;
        }

        if (const auto& normal_tex = material.getNormalTexture())
        {
            TextureHandle handle = rhi.loadTexture(normal_tex->path);
            m_normal_tex         = handle;
        }

        if (const auto& occlusion_tex = material.getOcclusionTexture())
        {
            TextureHandle handle = rhi.loadTexture(occlusion_tex->path);
            m_occlusion_tex      = handle;
        }

        if (const auto& emissive_tex = material.getEmissiveTexture())
        {
            TextureHandle handle = rhi.loadTexture(emissive_tex->path);
            m_emissive_tex       = handle;
        }

        m_render_state = material.getRenderState();
    }

    void RenderMaterial::disposal(RHI& rhi)
    {
        if (m_base_color_tex.has_value())
            rhi.deleteTexture(m_base_color_tex.value());
        if (m_metallic_roughness_tex.has_value())
            rhi.deleteTexture(m_metallic_roughness_tex.value());
        if (m_normal_tex.has_value())
            rhi.deleteTexture(m_normal_tex.value());
        if (m_occlusion_tex.has_value())
            rhi.deleteTexture(m_occlusion_tex.value());
        if (m_emissive_tex.has_value())
            rhi.deleteTexture(m_emissive_tex.value());

        m_base_color_tex         = std::nullopt;
        m_metallic_roughness_tex = std::nullopt;
        m_normal_tex             = std::nullopt;
        m_occlusion_tex          = std::nullopt;
        m_emissive_tex           = std::nullopt;
    }

    void RenderMaterial::bind() const
    {
        glUseProgram(m_shader);

        uint32_t tex_unit = 0;

        if (m_base_color_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_base_color_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "uBaseColorTexture"), tex_unit);
            tex_unit++;
        }

        if (m_metallic_roughness_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_metallic_roughness_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "uMetallicRoughnessTexture"), tex_unit);
            tex_unit++;
        }

        if (m_normal_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_normal_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "uNormalTexture"), tex_unit);
            tex_unit++;
        }

        if (m_occlusion_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_occlusion_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "uOcclusionTexture"), tex_unit);
            tex_unit++;
        }

        if (m_emissive_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_emissive_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "uEmissiveTexture"), tex_unit);
            tex_unit++;
        }

        glUniform4fv(glGetUniformLocation(m_shader, "uBaseColorFactor"), 1, &m_base_color_factor[0]);
        glUniform1f(glGetUniformLocation(m_shader, "uMetallicFactor"), m_metallic_factor);
        glUniform1f(glGetUniformLocation(m_shader, "uRoughnessFactor"), m_roughness_factor);
        glUniform3fv(glGetUniformLocation(m_shader, "uEmissiveFactor"), 1, &m_emissive_factor[0]);
    }

    void RenderMaterial::unbind() const { glUseProgram(0); }

} // namespace RealmEngine