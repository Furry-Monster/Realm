#include "renderer/ibl/hdr_texture.h"

#include "core/log/log_macros.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#  define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>

namespace RealmEngine
{
    HDRTexture::HDRTexture(RHIDevice& device, const std::string& path)
    {
        stbi_set_flip_vertically_on_load(true);

        int    width, height, num_channels;
        float* data = stbi_loadf(path.c_str(), &width, &height, &num_channels, 0);

        if (!data)
        {
            RE_LOG_ERROR("Failed to load HDR texture data: " + path);
            return;
        }

        TextureDesc desc;
        desc.type       = TextureType::Texture2D;
        desc.format     = TextureFormat::RGB16F;
        desc.width      = width;
        desc.height     = height;
        desc.min_filter = TextureFilter::Linear;
        desc.mag_filter = TextureFilter::Linear;
        desc.wrap_s     = TextureWrap::ClampToEdge;
        desc.wrap_t     = TextureWrap::ClampToEdge;
        desc.data       = data;

        m_texture = device.createTexture(desc);
        stbi_image_free(data);

        if (!m_texture)
            RE_LOG_ERROR("Failed to create HDR texture from: " + path);
    }
} // namespace RealmEngine
