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
        virtual ~RHIShader() noexcept = default;

        [[nodiscard]] virtual bool isValid() const = 0;
        virtual void               use()           = 0;

        virtual void setBool(const std::string& name, bool value)             = 0;
        virtual void setInt(const std::string& name, int value)               = 0;
        virtual void setFloat(const std::string& name, float value)           = 0;
        virtual void setVec2(const std::string& name, const glm::vec2& value) = 0;
        virtual void setVec3(const std::string& name, const glm::vec3& value) = 0;
        virtual void setIVec3(const std::string& name, const glm::ivec3& value) = 0;
        virtual void setVec4(const std::string& name, const glm::vec4& value) = 0;
        virtual void setMat3(const std::string& name, const glm::mat3& value) = 0;
        virtual void setMat4(const std::string& name, const glm::mat4& value) = 0;

        virtual void setVec3Array(const std::string& name, const std::vector<glm::vec3>& values) = 0;
        virtual void setFloatArray(const std::string& name, const std::vector<float>& values)    = 0;
        virtual void setIntArray(const std::string& name, const std::vector<int>& values)        = 0;

        virtual void setMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
        {
            setMat4("model", model);
            setMat4("view", view);
            setMat4("projection", projection);
            setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        }

        virtual void bindUniformBlock(const std::string& name, uint32_t binding_point)       = 0;
        virtual void bindShaderStorageBlock(const std::string& name, uint32_t binding_point) = 0;
    };

} // namespace RealmEngine
