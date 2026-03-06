#include "functional/render/light_probe_baker.h"

#include <glad/glad.h>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include "functional/render/light.h"
#include "functional/render/render_scene.h"
#include "functional/render/rhi/rhi_buffer.h"
#include "functional/render/rhi/rhi_device.h"
#include "functional/render/rhi/rhi_framebuffer.h"
#include "functional/render/rhi/rhi_shader.h"
#include "functional/render/rhi/rhi_texture.h"

namespace RealmEngine
{
    // SH basis constants
    static constexpr float kY00  = 0.282095f; // 0.5 * sqrt(1/PI)
    static constexpr float kY1x  = 0.488603f; // 0.5 * sqrt(3/PI)
    static constexpr float kY2xy = 1.092548f; // 0.5 * sqrt(15/PI)
    static constexpr float kY20  = 0.315392f; // 0.25 * sqrt(5/PI)
    static constexpr float kY2x2 = 0.546274f; // 0.25 * sqrt(15/PI)

    LightProbeBaker::LightProbeBaker(RHIDevice& device) : m_device(device)
    {
        FramebufferDesc desc;
        desc.width  = CUBEMAP_RESOLUTION;
        desc.height = CUBEMAP_RESOLUTION;

        FramebufferAttachment color;
        color.format           = TextureFormat::RGB16F;
        color.min_filter       = TextureFilter::Linear;
        color.mag_filter       = TextureFilter::Linear;
        color.wrap             = TextureWrap::ClampToEdge;
        color.is_cubemap       = true;
        desc.color_attachments = {color};

        FramebufferAttachment depth;
        depth.format          = TextureFormat::Depth24;
        depth.is_renderbuffer = true;
        desc.has_depth        = true;
        desc.depth_attachment = depth;

        m_cubemap_fbo = m_device.createFramebuffer(desc);
        m_light_ssbo  = m_device.createBuffer(BufferType::ShaderStorage, BufferUsage::Dynamic, nullptr, BUFFER_SIZE);
    }

    LightProbeBaker::~LightProbeBaker() noexcept = default;

    void LightProbeBaker::initShader(const std::string& shader_path)
    {
        m_shader =
            m_device.createShader(shader_path + "/builtin/probe_bake.vert", shader_path + "/builtin/probe_bake.frag");
        if (m_shader)
            m_shader->bindShaderStorageBlock("LightBuffer", 1);
    }

    LightProbeBaker::BakeResult LightProbeBaker::bake(const glm::vec3& position, RenderScene& scene)
    {
        BakeResult result;
        if (!m_shader || !m_shader->isValid())
            return result;

        // Upload light data for the bake shader
        if (m_light_ssbo)
        {
            const size_t count   = std::min(scene.getLights().size(), MAX_LIGHTS);
            const int    count_i = static_cast<int>(count);
            LightData    data[MAX_LIGHTS] {};
            for (size_t i = 0; i < count; ++i)
            {
                auto& l             = scene.getLights()[i];
                data[i].position    = glm::vec4(l.position, static_cast<float>(static_cast<int>(l.type)));
                data[i].direction   = glm::vec4(l.direction, l.intensity);
                data[i].color       = glm::vec4(l.color, l.constant);
                data[i].attenuation = glm::vec4(l.linear, l.quadratic, l.range, l.inner_cone_angle);
                data[i].spot_area   = glm::vec4(l.outer_cone_angle, l.width, l.height, 0.0f);
            }
            m_light_ssbo->setSubData(&count_i, 0, sizeof(int));
            m_light_ssbo->setSubData(data, 16, count * sizeof(LightData));
            m_light_ssbo->bindBase(1);
        }

        renderCubemap(position, scene);
        readCubemapPixels();
        result.sh_coefficients = projectToSH();
        result.success         = true;
        return result;
    }

    void LightProbeBaker::renderCubemap(const glm::vec3& position, RenderScene& scene)
    {
        const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);

        const glm::mat4 views[6] = {glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
                                    glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
                                    glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
                                    glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
                                    glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
                                    glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))};

        m_shader->use();

        for (int face = 0; face < 6; ++face)
        {
            m_cubemap_fbo->setCubeFace(face);
            m_cubemap_fbo->bind();

            m_device.setViewport(0, 0, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION);
            m_device.setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            m_device.clear(ClearFlags::Color | ClearFlags::Depth);
            m_device.setDepthTest(true);

            const auto& objects  = scene.getRenderObjects();
            const auto& matrices = scene.getRenderModelMatrices();
            for (size_t i = 0; i < objects.size(); ++i)
            {
                auto&     ro    = *objects[i];
                glm::mat4 model = (i < matrices.size()) ? matrices[i] : glm::mat4(1.0f);
                m_shader->setMVP(model, views[face], projection);
                ro.drawShadow(*m_shader);
            }
        }

        m_device.bindDefaultFramebuffer();
    }

    void LightProbeBaker::readCubemapPixels()
    {
        constexpr int face_pixels = CUBEMAP_RESOLUTION * CUBEMAP_RESOLUTION;
        m_pixel_buffer.resize(6 * face_pixels);

        for (int face = 0; face < 6; ++face)
        {
            m_cubemap_fbo->setCubeFace(face);
            m_cubemap_fbo->bind();

            std::vector<float> raw(face_pixels * 3);
            glReadPixels(0, 0, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION, GL_RGB, GL_FLOAT, raw.data());

            for (int p = 0; p < face_pixels; ++p)
                m_pixel_buffer[face * face_pixels + p] = glm::vec3(raw[p * 3], raw[p * 3 + 1], raw[p * 3 + 2]);
        }

        m_cubemap_fbo->unbind();
    }

    std::array<glm::vec3, 9> LightProbeBaker::projectToSH() const
    {
        std::array<glm::vec3, 9> coeffs {};

        int res = CUBEMAP_RESOLUTION;

        using BasisFn    = float (*)(const glm::vec3&);
        BasisFn basis[9] = {shBasis0, shBasis1, shBasis2, shBasis3, shBasis4, shBasis5, shBasis6, shBasis7, shBasis8};

        for (int face = 0; face < 6; ++face)
        {
            for (int y = 0; y < res; ++y)
            {
                for (int x = 0; x < res; ++x)
                {
                    float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(res) * 2.0f - 1.0f;
                    float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(res) * 2.0f - 1.0f;

                    glm::vec3 dir;
                    switch (face)
                    {
                        case 0:
                            dir = glm::vec3(1, -v, -u);
                            break; // +X
                        case 1:
                            dir = glm::vec3(-1, -v, u);
                            break; // -X
                        case 2:
                            dir = glm::vec3(u, 1, v);
                            break; // +Y
                        case 3:
                            dir = glm::vec3(u, -1, -v);
                            break; // -Y
                        case 4:
                            dir = glm::vec3(u, -v, 1);
                            break; // +Z
                        case 5:
                            dir = glm::vec3(-u, -v, -1);
                            break; // -Z
                        default:
                            break;
                    }
                    dir = glm::normalize(dir);

                    // Solid angle per cubemap texel: dw = 4 / (d^2 * sqrt(d^2) * res^2) where d^2 = u^2 + v^2 + 1
                    float dist2       = u * u + v * v + 1.0f;
                    float solid_angle = 4.0f / (dist2 * std::sqrt(dist2) * static_cast<float>(res * res));

                    glm::vec3 color = m_pixel_buffer[face * res * res + y * res + x];

                    for (size_t k = 0; k < 9; ++k)
                        coeffs[k] += color * basis[k](dir) * solid_angle;
                }
            }
        }

        // Cosine lobe convolution: A_0 = PI, A_1 = 2*PI/3, A_2 = PI/4
        static constexpr float PI     = 3.14159265358979323846f;
        static constexpr float A_l[3] = {PI, 2.0f * PI / 3.0f, PI / 4.0f};

        for (size_t k = 0; k < 9; ++k)
        {
            int band = (k < 1) ? 0 : (k < 4) ? 1 : 2;
            coeffs[k] *= A_l[band];
        }

        return coeffs;
    }

    // Band 0: Y_00
    float LightProbeBaker::shBasis0(const glm::vec3& /*d*/) { return kY00; }

    // Band 1: Y_1,-1 = c * y,  Y_1,0 = c * z,  Y_1,1 = c * x
    float LightProbeBaker::shBasis1(const glm::vec3& d) { return kY1x * d.y; }
    float LightProbeBaker::shBasis2(const glm::vec3& d) { return kY1x * d.z; }
    float LightProbeBaker::shBasis3(const glm::vec3& d) { return kY1x * d.x; }

    // Band 2: Y_2,-2 = c * xy,  Y_2,-1 = c * yz,  Y_2,0 = c * (3z^2 - 1)
    //          Y_2,1 = c * xz,  Y_2,2 = c * (x^2 - y^2)
    float LightProbeBaker::shBasis4(const glm::vec3& d) { return kY2xy * d.x * d.y; }
    float LightProbeBaker::shBasis5(const glm::vec3& d) { return kY2xy * d.y * d.z; }
    float LightProbeBaker::shBasis6(const glm::vec3& d) { return kY20 * (3.0f * d.z * d.z - 1.0f); }
    float LightProbeBaker::shBasis7(const glm::vec3& d) { return kY2xy * d.x * d.z; }
    float LightProbeBaker::shBasis8(const glm::vec3& d) { return kY2x2 * (d.x * d.x - d.y * d.y); }

} // namespace RealmEngine
