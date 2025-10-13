#include "render_material.h"
#include <glad/gl.h>

namespace RealmEngine
{
    void RenderMaterial::setShaderProgram(ProgramHandle shader) { m_shader = shader; }
    void RenderMaterial::setBaseColorTexture(TextureHandle handle) { m_base_color_tex = handle; }
    void RenderMaterial::setMetallicRoughnessTexture(TextureHandle handle) { m_metallic_roughness_tex = handle; }
    void RenderMaterial::setNormalTexture(TextureHandle handle) { m_normal_tex = handle; }
    void RenderMaterial::setOcclusionTexture(TextureHandle handle) { m_occlusion_tex = handle; }
    void RenderMaterial::setEmissiveTexture(TextureHandle handle) { m_emissive_tex = handle; }

    ProgramHandle                RenderMaterial::getShaderProgram() const { return m_shader; }
    std::optional<TextureHandle> RenderMaterial::getBaseColorTexture() const { return m_base_color_tex; }
    std::optional<TextureHandle> RenderMaterial::getMetallicRoughnessTexture() const
    {
        return m_metallic_roughness_tex;
    }
    std::optional<TextureHandle> RenderMaterial::getNormalTexture() const { return m_normal_tex; }
    std::optional<TextureHandle> RenderMaterial::getOcclusionTexture() const { return m_occlusion_tex; }
    std::optional<TextureHandle> RenderMaterial::getEmissiveTexture() const { return m_emissive_tex; }

    void RenderMaterial::setBaseColorFactor(const glm::vec4& color) { m_base_color_factor = color; }
    void RenderMaterial::setMetallicFactor(float metallic) { m_metallic_factor = metallic; }
    void RenderMaterial::setRoughnessFactor(float roughness) { m_roughness_factor = roughness; }
    void RenderMaterial::setEmissiveFactor(const glm::vec3& emissive) { m_emissive_factor = emissive; }

    const glm::vec4& RenderMaterial::getBaseColorFactor() const { return m_base_color_factor; }
    float            RenderMaterial::getMetallicFactor() const { return m_metallic_factor; }
    float            RenderMaterial::getRoughnessFactor() const { return m_roughness_factor; }
    const glm::vec3& RenderMaterial::getEmissiveFactor() const { return m_emissive_factor; }

    void               RenderMaterial::setRenderState(const RenderState& state) { m_render_state = state; }
    const RenderState& RenderMaterial::getRenderState() const { return m_render_state; }

    void RenderMaterial::bind() const
    {
        glUseProgram(m_shader);

        uint32_t tex_unit = 0;

        if (m_base_color_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_base_color_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "u_BaseColorTex"), tex_unit);
            tex_unit++;
        }

        if (m_metallic_roughness_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_metallic_roughness_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "u_MetallicRoughnessTex"), tex_unit);
            tex_unit++;
        }

        if (m_normal_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_normal_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "u_NormalTex"), tex_unit);
            tex_unit++;
        }

        if (m_occlusion_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_occlusion_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "u_OcclusionTex"), tex_unit);
            tex_unit++;
        }

        if (m_emissive_tex.has_value())
        {
            glActiveTexture(GL_TEXTURE0 + tex_unit);
            glBindTexture(GL_TEXTURE_2D, m_emissive_tex.value());
            glUniform1i(glGetUniformLocation(m_shader, "u_EmissiveTex"), tex_unit);
            tex_unit++;
        }

        glUniform4fv(glGetUniformLocation(m_shader, "u_BaseColorFactor"), 1, &m_base_color_factor[0]);
        glUniform1f(glGetUniformLocation(m_shader, "u_MetallicFactor"), m_metallic_factor);
        glUniform1f(glGetUniformLocation(m_shader, "u_RoughnessFactor"), m_roughness_factor);
        glUniform3fv(glGetUniformLocation(m_shader, "u_EmissiveFactor"), 1, &m_emissive_factor[0]);
    }

    void RenderMaterial::unbind() const { glUseProgram(0); }

} // namespace RealmEngine