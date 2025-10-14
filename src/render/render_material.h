#pragma once

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "render/rhi.h"
#include "resource/datatype/model/material.h"
#include <optional>

namespace RealmEngine
{
    class RenderMaterial
    {
    public:
        RenderMaterial()           = default;
        ~RenderMaterial() noexcept = default;

        RenderMaterial(const RenderMaterial& that)                = delete;
        RenderMaterial& operator=(const RenderMaterial& that)     = delete;
        RenderMaterial(RenderMaterial&& that) noexcept            = default;
        RenderMaterial& operator=(RenderMaterial&& that) noexcept = default;

        void          setShaderProgram(ProgramHandle shader);
        ProgramHandle getShaderProgram() const;

        void                         setBaseColorTexture(TextureHandle handle);
        void                         setMetallicRoughnessTexture(TextureHandle handle);
        void                         setNormalTexture(TextureHandle handle);
        void                         setOcclusionTexture(TextureHandle handle);
        void                         setEmissiveTexture(TextureHandle handle);
        std::optional<TextureHandle> getBaseColorTexture() const;
        std::optional<TextureHandle> getMetallicRoughnessTexture() const;
        std::optional<TextureHandle> getNormalTexture() const;
        std::optional<TextureHandle> getOcclusionTexture() const;
        std::optional<TextureHandle> getEmissiveTexture() const;

        void             setBaseColorFactor(const glm::vec4& color);
        void             setMetallicFactor(float metallic);
        void             setRoughnessFactor(float roughness);
        void             setEmissiveFactor(const glm::vec3& emissive);
        const glm::vec4& getBaseColorFactor() const;
        float            getMetallicFactor() const;
        float            getRoughnessFactor() const;
        const glm::vec3& getEmissiveFactor() const;

        void               setRenderState(const RenderState& state);
        const RenderState& getRenderState() const;

        void bind() const;
        void unbind() const;

    private:
        ProgramHandle m_shader {0};

        std::optional<TextureHandle> m_base_color_tex;
        std::optional<TextureHandle> m_metallic_roughness_tex;
        std::optional<TextureHandle> m_normal_tex;
        std::optional<TextureHandle> m_occlusion_tex;
        std::optional<TextureHandle> m_emissive_tex;

        glm::vec4 m_base_color_factor {1.0f, 1.0f, 1.0f, 1.0f};
        float     m_metallic_factor {1.0f};
        float     m_roughness_factor {1.0f};
        glm::vec3 m_emissive_factor {0.0f, 0.0f, 0.0f};

        RenderState m_render_state;
    };
} // namespace RealmEngine
