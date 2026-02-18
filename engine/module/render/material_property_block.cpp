#include "module/render/material_property_block.h"

#include "module/render/rhi/rhi_shader.h"
#include "module/render/rhi/rhi_texture.h"

namespace RealmEngine
{
    MaterialProperty* MaterialPropertyBlock::find(const std::string& name)
    {
        for (auto& [k, v] : m_props)
            if (k == name)
                return &v;
        return nullptr;
    }

    const MaterialProperty* MaterialPropertyBlock::find(const std::string& name) const
    {
        for (const auto& [k, v] : m_props)
            if (k == name)
                return &v;
        return nullptr;
    }

    void MaterialPropertyBlock::setBool(const std::string& name, const bool v)
    {
        auto* p = find(name);
        if (p)
        {
            p->type      = PropType::Bool;
            p->values[0] = v ? 1.0f : 0.0f;
            return;
        }
        MaterialProperty prop;
        prop.type      = PropType::Bool;
        prop.values[0] = v ? 1.0f : 0.0f;
        m_props.emplace_back(name, prop);
    }

    void MaterialPropertyBlock::setFloat(const std::string& name, const float v)
    {
        auto* p = find(name);
        if (p)
        {
            p->type      = PropType::Float;
            p->values[0] = v;
            return;
        }
        MaterialProperty prop;
        prop.type      = PropType::Float;
        prop.values[0] = v;
        m_props.emplace_back(name, prop);
    }

    void MaterialPropertyBlock::setInt(const std::string& name, const int v)
    {
        auto* p = find(name);
        if (p)
        {
            p->type      = PropType::Int;
            p->values[0] = static_cast<float>(v);
            return;
        }
        MaterialProperty prop;
        prop.type      = PropType::Int;
        prop.values[0] = static_cast<float>(v);
        m_props.emplace_back(name, prop);
    }

    void MaterialPropertyBlock::setVec2(const std::string& name, const glm::vec2& v)
    {
        auto* p = find(name);
        if (p)
        {
            p->type      = PropType::Vec2;
            p->values[0] = v.x;
            p->values[1] = v.y;
            return;
        }
        MaterialProperty prop;
        prop.type      = PropType::Vec2;
        prop.values[0] = v.x;
        prop.values[1] = v.y;
        m_props.emplace_back(name, prop);
    }

    void MaterialPropertyBlock::setVec3(const std::string& name, const glm::vec3& v)
    {
        auto* p = find(name);
        if (p)
        {
            p->type      = PropType::Vec3;
            p->values[0] = v.x;
            p->values[1] = v.y;
            p->values[2] = v.z;
            return;
        }
        MaterialProperty prop;
        prop.type      = PropType::Vec3;
        prop.values[0] = v.x;
        prop.values[1] = v.y;
        prop.values[2] = v.z;
        m_props.emplace_back(name, prop);
    }

    void MaterialPropertyBlock::setVec4(const std::string& name, const glm::vec4& v)
    {
        auto* p = find(name);
        if (p)
        {
            p->type      = PropType::Vec4;
            p->values[0] = v.x;
            p->values[1] = v.y;
            p->values[2] = v.z;
            p->values[3] = v.w;
            return;
        }
        MaterialProperty prop;
        prop.type      = PropType::Vec4;
        prop.values[0] = v.x;
        prop.values[1] = v.y;
        prop.values[2] = v.z;
        prop.values[3] = v.w;
        m_props.emplace_back(name, prop);
    }

    void MaterialPropertyBlock::setTexture(const std::string& name, std::shared_ptr<RHITexture> tex, const int unit)
    {
        auto* p = find(name);
        if (p)
        {
            p->type         = PropType::Texture2D;
            p->texture      = std::move(tex);
            p->texture_unit = unit;
            return;
        }
        MaterialProperty prop;
        prop.type         = PropType::Texture2D;
        prop.texture      = std::move(tex);
        prop.texture_unit = unit;
        m_props.emplace_back(name, prop);
    }

    bool MaterialPropertyBlock::getBool(const std::string& name, const bool default_val) const
    {
        const auto* p = find(name);
        return p ? (p->values[0] != 0.0f) : default_val;
    }

    float MaterialPropertyBlock::getFloat(const std::string& name, const float default_val) const
    {
        const auto* p = find(name);
        return p ? p->values[0] : default_val;
    }

    int MaterialPropertyBlock::getInt(const std::string& name, const int default_val) const
    {
        const auto* p = find(name);
        return p ? static_cast<int>(p->values[0]) : default_val;
    }

    glm::vec2 MaterialPropertyBlock::getVec2(const std::string& name, const glm::vec2& default_val) const
    {
        const auto* p = find(name);
        return p ? glm::vec2(p->values[0], p->values[1]) : default_val;
    }

    glm::vec3 MaterialPropertyBlock::getVec3(const std::string& name, const glm::vec3& default_val) const
    {
        const auto* p = find(name);
        return p ? glm::vec3(p->values[0], p->values[1], p->values[2]) : default_val;
    }

    glm::vec4 MaterialPropertyBlock::getVec4(const std::string& name, const glm::vec4& default_val) const
    {
        const auto* p = find(name);
        return p ? glm::vec4(p->values[0], p->values[1], p->values[2], p->values[3]) : default_val;
    }

    std::shared_ptr<RHITexture> MaterialPropertyBlock::getTexture(const std::string& name) const
    {
        const auto* p = find(name);
        return (p && p->type == PropType::Texture2D) ? p->texture : nullptr;
    }

    bool MaterialPropertyBlock::has(const std::string& name) const { return find(name) != nullptr; }

    void MaterialPropertyBlock::remove(const std::string& name)
    {
        for (auto it = m_props.begin(); it != m_props.end(); ++it)
        {
            if (it->first == name)
            {
                m_props.erase(it);
                return;
            }
        }
    }

    void MaterialPropertyBlock::clear() { m_props.clear(); }

    void MaterialPropertyBlock::applyToShader(RHIShader& shader) const
    {
        for (const auto& [name, prop] : m_props)
        {
            switch (prop.type)
            {
                case PropType::Bool:
                    shader.setBool(name, prop.values[0] != 0.0f);
                    break;
                case PropType::Float:
                    shader.setFloat(name, prop.values[0]);
                    break;
                case PropType::Int:
                    shader.setInt(name, static_cast<int>(prop.values[0]));
                    break;
                case PropType::Vec2:
                    shader.setVec2(name, glm::vec2(prop.values[0], prop.values[1]));
                    break;
                case PropType::Vec3:
                    shader.setVec3(name, glm::vec3(prop.values[0], prop.values[1], prop.values[2]));
                    break;
                case PropType::Vec4:
                    shader.setVec4(name, glm::vec4(prop.values[0], prop.values[1], prop.values[2], prop.values[3]));
                    break;
                case PropType::Texture2D:
                    if (prop.texture && prop.texture_unit >= 0)
                    {
                        prop.texture->bind(static_cast<uint32_t>(prop.texture_unit));
                        shader.setInt(name, prop.texture_unit);
                    }
                    break;
            }
        }
    }

} // namespace RealmEngine
