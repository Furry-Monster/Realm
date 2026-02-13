#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace RealmEngine
{
    class RHIShader
    {
    public:
        virtual ~RHIShader() = default;

        virtual void use() = 0;

        // Uniform setters
        virtual void setBool(const std::string& name, bool value)             = 0;
        virtual void setInt(const std::string& name, int value)               = 0;
        virtual void setFloat(const std::string& name, float value)           = 0;
        virtual void setVec2(const std::string& name, const glm::vec2& value) = 0;
        virtual void setVec3(const std::string& name, const glm::vec3& value) = 0;
        virtual void setMat4(const std::string& name, const glm::mat4& value) = 0;

        virtual void setVec3Array(const std::string& name, const std::vector<glm::vec3>& values) = 0;
        virtual void setFloatArray(const std::string& name, const std::vector<float>& values)    = 0;
        virtual void setIntArray(const std::string& name, const std::vector<int>& values)        = 0;

        // Convenience: set model / view / projection in one call
        virtual void setMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) = 0;

        // Uniform block binding
        virtual void bindUniformBlock(const std::string& name, uint32_t binding_point) = 0;
    };

} // namespace RealmEngine
