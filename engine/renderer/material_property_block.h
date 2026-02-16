#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace RealmEngine
{
    class RHIShader;
    class RHITexture;

    enum class PropType : uint8_t
    {
        Bool,
        Float,
        Int,
        Vec2,
        Vec3,
        Vec4,
        Texture2D
    };

    struct MaterialProperty
    {
        PropType                    type = PropType::Float;
        float                       values[4] {};
        std::shared_ptr<RHITexture> texture;
        int                         texture_unit = -1;
    };

    class MaterialPropertyBlock
    {
    public:
        void setBool(const std::string& name, bool v);
        void setFloat(const std::string& name, float v);
        void setInt(const std::string& name, int v);
        void setVec2(const std::string& name, const glm::vec2& v);
        void setVec3(const std::string& name, const glm::vec3& v);
        void setVec4(const std::string& name, const glm::vec4& v);
        void setTexture(const std::string& name, std::shared_ptr<RHITexture> tex, int unit);

        [[nodiscard]] bool      getBool(const std::string& name, bool default_val = false) const;
        [[nodiscard]] float     getFloat(const std::string& name, float default_val = 0.0f) const;
        [[nodiscard]] int       getInt(const std::string& name, int default_val = 0) const;
        [[nodiscard]] glm::vec2 getVec2(const std::string& name, const glm::vec2& default_val = glm::vec2(0)) const;
        [[nodiscard]] glm::vec3 getVec3(const std::string& name, const glm::vec3& default_val = glm::vec3(0)) const;
        [[nodiscard]] glm::vec4 getVec4(const std::string& name, const glm::vec4& default_val = glm::vec4(0)) const;
        [[nodiscard]] std::shared_ptr<RHITexture> getTexture(const std::string& name) const;

        [[nodiscard]] bool has(const std::string& name) const;
        void               remove(const std::string& name);
        void               clear();

        void applyToShader(RHIShader& shader) const;

        using PropertyMap = std::vector<std::pair<std::string, MaterialProperty>>;
        [[nodiscard]] const PropertyMap& getProperties() const { return m_props; }
        [[nodiscard]] PropertyMap&       getProperties() { return m_props; }

    private:
        MaterialProperty*       find(const std::string& name);
        const MaterialProperty* find(const std::string& name) const;

        PropertyMap m_props;
    };

} // namespace RealmEngine
