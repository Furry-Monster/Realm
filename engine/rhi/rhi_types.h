#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace RealmEngine
{
    // ---- Buffer ----------------------------------------------------------

    enum class BufferType : uint8_t
    {
        Vertex,
        Index,
        Uniform
    };

    enum class BufferUsage : uint8_t
    {
        Static,  // uploaded once
        Dynamic, // updated frequently
        Stream   // updated every frame
    };

    // ---- Texture ---------------------------------------------------------

    enum class TextureFormat : uint8_t
    {
        R8,
        RG8,
        RGB8,
        RGBA8,
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
        Depth16,
        Depth24,
        Depth32F,
        Depth24Stencil8
    };

    enum class TextureType : uint8_t
    {
        Texture2D,
        TextureCube
    };

    enum class TextureWrap : uint8_t
    {
        Repeat,
        ClampToEdge,
        ClampToBorder,
        MirroredRepeat
    };

    enum class TextureFilter : uint8_t
    {
        Nearest,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear
    };

    struct TextureDesc
    {
        TextureType   type       = TextureType::Texture2D;
        TextureFormat format     = TextureFormat::RGBA8;
        int           width      = 0;
        int           height     = 0;
        TextureFilter min_filter = TextureFilter::Linear;
        TextureFilter mag_filter = TextureFilter::Linear;
        TextureWrap   wrap_s     = TextureWrap::ClampToEdge;
        TextureWrap   wrap_t     = TextureWrap::ClampToEdge;
        TextureWrap   wrap_r     = TextureWrap::ClampToEdge;
        bool          gen_mips   = false;
        const void*   data       = nullptr;
    };

    // ---- Framebuffer -----------------------------------------------------

    struct FramebufferAttachment
    {
        TextureFormat format          = TextureFormat::RGBA8;
        TextureFilter min_filter      = TextureFilter::Linear;
        TextureFilter mag_filter      = TextureFilter::Linear;
        TextureWrap   wrap            = TextureWrap::ClampToEdge;
        bool          is_renderbuffer = false; // depth/stencil only
        bool          gen_mips        = false;
    };

    struct FramebufferDesc
    {
        int                                width  = 0;
        int                                height = 0;
        std::vector<FramebufferAttachment> color_attachments;
        FramebufferAttachment              depth_attachment;
        bool                               has_depth = false;
    };

    // ---- Vertex input ----------------------------------------------------

    struct VertexAttribute
    {
        uint32_t location;
        uint32_t component_count; // 1-4
        size_t   offset;
        bool     normalized = false;
    };

    struct VertexLayout
    {
        size_t                       stride = 0;
        std::vector<VertexAttribute> attributes;
    };

    // ---- Render state ----------------------------------------------------

    enum class PrimitiveType : uint8_t
    {
        Triangles,
        TriangleStrip,
        Lines,
        Points
    };

    enum class DepthFunc : uint8_t
    {
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
        Equal,
        NotEqual,
        Always,
        Never
    };

    enum class CullFace : uint8_t
    {
        Front,
        Back,
        FrontAndBack,
        None
    };

    enum class ClearFlags : uint32_t
    {
        Color   = 1u << 0,
        Depth   = 1u << 1,
        Stencil = 1u << 2
    };

    inline ClearFlags operator|(ClearFlags a, ClearFlags b)
    {
        return static_cast<ClearFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline bool operator&(ClearFlags a, ClearFlags b)
    {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    // ---- Bloom direction (used by BloomPass) ------------------------------

    enum class BloomDirection : uint8_t
    {
        BOTH       = 0,
        HORIZONTAL = 1,
        VERTICAL   = 2
    };

} // namespace RealmEngine
