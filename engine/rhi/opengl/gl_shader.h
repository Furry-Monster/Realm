#pragma once

#include <cstdint>
#include <string>

#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    class GLShader final : public RHIShader
    {
    public:
        GLShader(const std::string& vertex_path, const std::string& fragment_path);
        GLShader(const std::string& vertex_path, const std::string& geometry_path, const std::string& fragment_path);
        ~GLShader() override;

        GLShader(const GLShader&)            = delete;
        GLShader& operator=(const GLShader&) = delete;

        bool isValid() const override { return m_id != 0; }

        void use() override;

        void setBool(const std::string& name, bool value) override;
        void setInt(const std::string& name, int value) override;
        void setFloat(const std::string& name, float value) override;
        void setVec2(const std::string& name, const glm::vec2& value) override;
        void setVec3(const std::string& name, const glm::vec3& value) override;
        void setVec4(const std::string& name, const glm::vec4& value) override;
        void setMat3(const std::string& name, const glm::mat3& value) override;
        void setMat4(const std::string& name, const glm::mat4& value) override;

        void setVec3Array(const std::string& name, const std::vector<glm::vec3>& values) override;
        void setFloatArray(const std::string& name, const std::vector<float>& values) override;
        void setIntArray(const std::string& name, const std::vector<int>& values) override;

        void bindUniformBlock(const std::string& name, uint32_t binding_point) override;

        uint32_t getNativeHandle() const { return m_id; }

    private:
        uint32_t    compileStage(uint32_t stage_type, const std::string& source, const std::string& path);
        bool        linkProgram(uint32_t vertex, uint32_t fragment, uint32_t geometry = 0);
        std::string loadFile(const std::string& path);

        uint32_t m_id {0};
    };

} // namespace RealmEngine
