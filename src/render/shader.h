#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace RealmEngine
{
    class Shader
    {
    public:
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader() noexcept;

        Shader(const Shader&)                = delete;
        Shader& operator=(const Shader&)     = delete;
        Shader(Shader&&) noexcept            = default;
        Shader& operator=(Shader&&) noexcept = default;

        void use() const;

        void setBool(const std::string& name, bool value) const;
        void setInt(const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;
        void setVec2(const std::string& name, const glm::vec2& value) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setVec3Array(const std::string& name, const std::vector<glm::vec3>& values) const;
        void setMat4(const std::string& name, const glm::mat4& value) const;
        void setModelViewProjectionMatrices(const glm::mat4& model,
                                            const glm::mat4& view,
                                            const glm::mat4& projection) const;

        unsigned int getId() const { return m_id; }

    private:
        unsigned int m_id;
    };
} // namespace RealmEngine
