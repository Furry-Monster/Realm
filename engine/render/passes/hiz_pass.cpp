#include "render/passes/hiz_pass.h"

#include <algorithm>
#include <cmath>

#include "render/rhi/rhi_device.h"
#include "render/rhi/rhi_framebuffer.h"
#include "render/rhi/rhi_shader.h"
#include "render/rhi/rhi_texture.h"
#include "render/rhi/rhi_types.h"
#include "render/scene_color_source.h"

namespace RealmEngine
{
    HiZPass::~HiZPass() noexcept = default;

    HiZPass::HiZPass(const std::string& shader_path) : RenderPass("hiz"), m_shader_path(shader_path) {}

    void HiZPass::init(RHIDevice& device)
    {
        m_compute_shader = device.createComputeShader(m_shader_path + "/builtin/hiz_generate.comp");
    }

    void HiZPass::execute(const RenderContext& ctx)
    {
        if (!m_scene_color)
            return;

        auto* scene_fb = m_scene_color->getFramebuffer();
        if (!scene_fb)
            return;

        auto* depth_tex = scene_fb->getDepthAttachment();
        if (!depth_tex)
            return;

        ensureTexture(*ctx.device, ctx.viewport_width, ctx.viewport_height);

        // Mip 0: sample scene depth via sampler, write to Hi-Z mip 0 via image store.
        m_compute_shader->use();

        ctx.device->bindTexture(0, *depth_tex);
        m_compute_shader->setInt("uDepthTexture", 0);
        m_hiz_texture->bindImage(1, 0, false, TextureAccess::WriteOnly);
        m_compute_shader->setInt("uDstImage", 1);
        m_compute_shader->setInt("uMipLevel", -1);
        m_compute_shader->setInt("uSrcWidth", m_tex_width);
        m_compute_shader->setInt("uSrcHeight", m_tex_height);

        uint32_t groups_x = (static_cast<uint32_t>(m_tex_width) + 7u) / 8u;
        uint32_t groups_y = (static_cast<uint32_t>(m_tex_height) + 7u) / 8u;
        ctx.device->dispatchCompute(groups_x, groups_y, 1);
        ctx.device->memoryBarrier(BarrierFlags::ImageAccess | BarrierFlags::TextureFetch);

        // Downsample each subsequent mip level.
        for (int mip = 1; mip < m_mip_count; ++mip)
        {
            const int src_w = std::max(1, m_tex_width >> (mip - 1));
            const int src_h = std::max(1, m_tex_height >> (mip - 1));
            const int dst_w = std::max(1, m_tex_width >> mip);
            const int dst_h = std::max(1, m_tex_height >> mip);

            m_hiz_texture->bindImage(0, mip - 1, false, TextureAccess::ReadOnly);
            m_hiz_texture->bindImage(1, mip, false, TextureAccess::WriteOnly);

            m_compute_shader->use();
            m_compute_shader->setInt("uSrcImage", 0);
            m_compute_shader->setInt("uDstImage", 1);
            m_compute_shader->setInt("uMipLevel", mip);
            m_compute_shader->setInt("uSrcWidth", src_w);
            m_compute_shader->setInt("uSrcHeight", src_h);

            groups_x = (static_cast<uint32_t>(dst_w) + 7u) / 8u;
            groups_y = (static_cast<uint32_t>(dst_h) + 7u) / 8u;
            ctx.device->dispatchCompute(groups_x, groups_y, 1);
            ctx.device->memoryBarrier(BarrierFlags::ImageAccess | BarrierFlags::TextureFetch);
        }
    }

    void HiZPass::dispose()
    {
        m_hiz_texture.reset();
        m_compute_shader.reset();
        m_tex_width  = 0;
        m_tex_height = 0;
        m_mip_count  = 0;
    }

    void HiZPass::ensureTexture(RHIDevice& device, const int width, const int height)
    {
        if (m_hiz_texture && m_tex_width == width && m_tex_height == height)
            return;

        m_tex_width  = width;
        m_tex_height = height;
        m_mip_count  = computeMipCount(width, height);

        TextureDesc td;
        td.type       = TextureType::Texture2D;
        td.format     = TextureFormat::R32F;
        td.width      = width;
        td.height     = height;
        td.min_filter = TextureFilter::NearestMipmapNearest;
        td.mag_filter = TextureFilter::Nearest;
        td.wrap_s     = TextureWrap::ClampToEdge;
        td.wrap_t     = TextureWrap::ClampToEdge;
        td.gen_mips   = true;

        m_hiz_texture = device.createTexture(td);
    }

    int HiZPass::computeMipCount(const int width, const int height) const
    {
        return 1 + static_cast<int>(std::floor(std::log2(static_cast<double>(std::max(width, height)))));
    }

} // namespace RealmEngine
