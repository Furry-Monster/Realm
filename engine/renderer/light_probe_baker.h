#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace RealmEngine
{
    class RHIDevice;
    class RHIShader;
    class RHIBuffer;
    class RHIFramebuffer;
    class RenderScene;

    class LightProbeBaker
    {
    public:
        static constexpr int CUBEMAP_RESOLUTION = 64;

        struct BakeResult
        {
            std::array<glm::vec3, 9> sh_coefficients {};
            bool                     success {false};
        };

        explicit LightProbeBaker(RHIDevice& device);
        ~LightProbeBaker() noexcept;

        LightProbeBaker(const LightProbeBaker&)            = delete;
        LightProbeBaker& operator=(const LightProbeBaker&) = delete;
        LightProbeBaker(LightProbeBaker&&)                 = delete;
        LightProbeBaker& operator=(LightProbeBaker&&)      = delete;

        void initShader(const std::string& shader_path);

        BakeResult bake(const glm::vec3& position, RenderScene& scene);

    private:
        void renderCubemap(const glm::vec3& position, RenderScene& scene);
        void readCubemapPixels();

        std::array<glm::vec3, 9> projectToSH() const;

        // SH basis functions evaluated at direction d
        static float shBasis0(const glm::vec3& d); // Y_00
        static float shBasis1(const glm::vec3& d); // Y_1,-1
        static float shBasis2(const glm::vec3& d); // Y_1,0
        static float shBasis3(const glm::vec3& d); // Y_1,1
        static float shBasis4(const glm::vec3& d); // Y_2,-2
        static float shBasis5(const glm::vec3& d); // Y_2,-1
        static float shBasis6(const glm::vec3& d); // Y_2,0
        static float shBasis7(const glm::vec3& d); // Y_2,1
        static float shBasis8(const glm::vec3& d); // Y_2,2

        RHIDevice& m_device;

        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_cubemap_fbo;
        std::unique_ptr<RHIBuffer>      m_light_ssbo;

        std::vector<glm::vec3> m_pixel_buffer;
    };

} // namespace RealmEngine
