#pragma once

#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
namespace RealmEngine
{
    using MaterialParamVal = std::variant<float, int, bool, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

    struct TextureRef
    {
        std::string path;
        std::string sampler_name;
        uint32_t    slot {0};
        bool        srgb = true;
    };

    struct RenderState
    {
        enum class BlendMode : uint8_t
        {
            Opaque,
            AlphaBlend,
            Additive,
            Multiply,
        };

        enum class CullMode : uint8_t
        {
            None,
            Front,
            Back,
        };

        enum class DepthTest : uint8_t
        {
            Always,
            Less,
            LessEqual,
            Greater,
            Equal,
        };

        BlendMode blend_mode {BlendMode::Opaque};
        CullMode  cull_mode {CullMode::Back};
        DepthTest depth_test {DepthTest::Less};
        bool      depth_write {true};
        bool      wireframe {false};
    };

    class Material
    {
    public:
        Material() = default;
        explicit Material(const std::string& shader_name = "default");
        ~Material() noexcept = default;

        Material(const Material& that)            = delete;
        Material(Material&& that)                 = delete;
        Material& operator=(const Material& that) = delete;
        Material& operator=(Material&& that)      = delete;

    private:
        std::string                                       m_shader_name;
        std::unordered_map<std::string, MaterialParamVal> m_params;
        std::unordered_map<std::string, TextureRef>       m_textures;
        RenderState                                       m_render_state;
    };
} // namespace RealmEngine