#include "functional/render/rhi/opengl/gl_shader.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include "core/base/macros.h"

namespace RealmEngine
{
    // ----- helpers --------------------------------------------------------

    std::string GLShader::loadFile(const std::string& path)
    {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            file.open(path);
            std::stringstream ss;
            ss << file.rdbuf();
            file.close();

            std::string source   = ss.str();
            std::string base_dir = std::filesystem::path(path).parent_path().string();

            std::unordered_set<std::string> included;
            included.insert(std::filesystem::canonical(path).string());
            return resolveIncludes(source, base_dir, included);
        }
        catch (const std::ifstream::failure&)
        {
            RE_LOG_ERROR("Failed to read shader file: " + path);
            return {};
        }
    }

    // Recursively resolve #include "path" directives
    std::string GLShader::resolveIncludes(const std::string&               source,
                                          const std::string&               base_dir,
                                          std::unordered_set<std::string>& included)
    {
        std::istringstream stream(source);
        std::ostringstream result;
        std::string        line;

        while (std::getline(stream, line))
        {
            // Match: #include "relative/path.glsl"
            auto pos = line.find("#include");
            if (pos != std::string::npos)
            {
                auto q1 = line.find('"', pos);
                auto q2 = line.find('"', q1 + 1);
                if (q1 != std::string::npos && q2 != std::string::npos)
                {
                    std::string           inc_path  = line.substr(q1 + 1, q2 - q1 - 1);
                    std::filesystem::path full_path = std::filesystem::path(base_dir) / inc_path;

                    try
                    {
                        std::string canonical = std::filesystem::canonical(full_path).string();
                        if (included.count(canonical))
                        {
                            result << "// [include guard] " << inc_path << "\n";
                            continue;
                        }
                        included.insert(canonical);

                        std::ifstream inc_file(canonical);
                        if (inc_file.is_open())
                        {
                            std::stringstream ss;
                            ss << inc_file.rdbuf();
                            std::string inc_dir = std::filesystem::path(canonical).parent_path().string();
                            result << resolveIncludes(ss.str(), inc_dir, included);
                            continue;
                        }
                    }
                    catch (const std::filesystem::filesystem_error&)
                    {}

                    RE_LOG_ERROR("Failed to resolve shader include: " + inc_path + " (from " + base_dir + ")");
                    result << "// [include failed] " << inc_path << "\n";
                    continue;
                }
            }
            result << line << "\n";
        }

        return result.str();
    }

    uint32_t GLShader::compileStage(uint32_t stage_type, const std::string& source, const std::string& path)
    {
        if (source.empty())
            return 0;

        uint32_t    id   = glCreateShader(stage_type);
        const char* code = source.c_str();
        glShaderSource(id, 1, &code, nullptr);
        glCompileShader(id);

        int  success;
        char info[512];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 512, nullptr, info);
            RE_LOG_ERROR("Shader compilation failed (" + path + "): " + std::string(info));
            glDeleteShader(id);
            return 0;
        }
        return id;
    }

    bool GLShader::linkProgram(uint32_t vertex, uint32_t fragment, uint32_t geometry)
    {
        m_id = glCreateProgram();
        glAttachShader(m_id, vertex);
        if (geometry != 0)
            glAttachShader(m_id, geometry);
        glAttachShader(m_id, fragment);
        glLinkProgram(m_id);

        int  success;
        char info[512];
        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_id, 512, nullptr, info);
            RE_LOG_ERROR("Shader link failed: " + std::string(info));
            glDeleteProgram(m_id);
            m_id = 0;
            return false;
        }
        return true;
    }

    // ----- constructors / destructor -------------------------------------

    GLShader::GLShader(const std::string& vertex_path, const std::string& fragment_path)
    {
        auto vs_src = loadFile(vertex_path);
        auto fs_src = loadFile(fragment_path);
        if (vs_src.empty() || fs_src.empty())
        {
            m_id = 0;
            return;
        }

        uint32_t vs = compileStage(GL_VERTEX_SHADER, vs_src, vertex_path);
        uint32_t fs = compileStage(GL_FRAGMENT_SHADER, fs_src, fragment_path);
        if (vs == 0 || fs == 0)
        {
            glDeleteShader(vs);
            glDeleteShader(fs);
            m_id = 0;
            return;
        }

        linkProgram(vs, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    GLShader::GLShader(const std::string& vertex_path,
                       const std::string& geometry_path,
                       const std::string& fragment_path)
    {
        auto vs_src = loadFile(vertex_path);
        auto gs_src = loadFile(geometry_path);
        auto fs_src = loadFile(fragment_path);
        if (vs_src.empty() || gs_src.empty() || fs_src.empty())
        {
            m_id = 0;
            return;
        }

        uint32_t vs = compileStage(GL_VERTEX_SHADER, vs_src, vertex_path);
        uint32_t gs = compileStage(GL_GEOMETRY_SHADER, gs_src, geometry_path);
        uint32_t fs = compileStage(GL_FRAGMENT_SHADER, fs_src, fragment_path);
        if (vs == 0 || gs == 0 || fs == 0)
        {
            glDeleteShader(vs);
            glDeleteShader(gs);
            glDeleteShader(fs);
            m_id = 0;
            return;
        }

        linkProgram(vs, fs, gs);
        glDeleteShader(vs);
        glDeleteShader(gs);
        glDeleteShader(fs);
    }

    GLShader::GLShader(const std::string& compute_path, ComputeTag)
    {
        auto cs_src = loadFile(compute_path);
        if (cs_src.empty())
        {
            m_id = 0;
            return;
        }

        uint32_t cs = compileStage(GL_COMPUTE_SHADER, cs_src, compute_path);
        if (cs == 0)
        {
            m_id = 0;
            return;
        }

        m_id = glCreateProgram();
        glAttachShader(m_id, cs);
        glLinkProgram(m_id);

        int  success;
        char info[512];
        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_id, 512, nullptr, info);
            RE_LOG_ERROR("Compute shader link failed: " + std::string(info));
            glDeleteProgram(m_id);
            m_id = 0;
        }
        glDeleteShader(cs);
    }

    GLShader::~GLShader() noexcept
    {
        if (m_id != 0)
            glDeleteProgram(m_id);
    }

    // ----- interface ------------------------------------------------------

#ifndef NDEBUG
    // Debug helper: verify this shader program is currently active before setting uniforms.
    // glUniform* operates on the *currently bound* program, not the one queried for location.
    static void assertProgramActive([[maybe_unused]] uint32_t expected_id)
    {
        GLint current = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current);
        if (static_cast<uint32_t>(current) != expected_id)
            RE_LOG_WARN("Setting uniform on shader " + std::to_string(expected_id) + " but active program is " +
                        std::to_string(current) + ". Call use() first.");
    }
#  define RE_ASSERT_SHADER_ACTIVE() assertProgramActive(m_id)
#else
#  define RE_ASSERT_SHADER_ACTIVE() ((void)0)
#endif

    void GLShader::use() { glUseProgram(m_id); }

    int GLShader::getUniformLocation(const std::string& name)
    {
        auto it = m_uniform_cache.find(name);
        if (it != m_uniform_cache.end())
            return it->second;
        int loc               = glGetUniformLocation(m_id, name.c_str());
        m_uniform_cache[name] = loc;
        return loc;
    }

    void GLShader::setBool(const std::string& name, bool value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniform1i(getUniformLocation(name), static_cast<int>(value));
    }

    void GLShader::setInt(const std::string& name, int value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniform1i(getUniformLocation(name), value);
    }

    void GLShader::setFloat(const std::string& name, float value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniform1f(getUniformLocation(name), value);
    }

    void GLShader::setVec2(const std::string& name, const glm::vec2& value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniform2f(getUniformLocation(name), value.x, value.y);
    }

    void GLShader::setVec3(const std::string& name, const glm::vec3& value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniform3f(getUniformLocation(name), value.x, value.y, value.z);
    }

    void GLShader::setVec4(const std::string& name, const glm::vec4& value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w);
    }

    void GLShader::setMat3(const std::string& name, const glm::mat3& value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void GLShader::setMat4(const std::string& name, const glm::mat4& value)
    {
        RE_ASSERT_SHADER_ACTIVE();
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void GLShader::setVec3Array(const std::string& name, const std::vector<glm::vec3>& values)
    {
        if (values.empty())
            return;
        glUniform3fv(getUniformLocation(name), static_cast<GLsizei>(values.size()), &values[0][0]);
    }

    void GLShader::setFloatArray(const std::string& name, const std::vector<float>& values)
    {
        glUniform1fv(getUniformLocation(name), static_cast<GLsizei>(values.size()), values.data());
    }

    void GLShader::setIntArray(const std::string& name, const std::vector<int>& values)
    {
        glUniform1iv(getUniformLocation(name), static_cast<GLsizei>(values.size()), values.data());
    }

    void GLShader::bindUniformBlock(const std::string& name, uint32_t binding_point)
    {
        uint32_t idx = glGetUniformBlockIndex(m_id, name.c_str());
        if (idx != GL_INVALID_INDEX)
            glUniformBlockBinding(m_id, idx, binding_point);
    }

    void GLShader::bindShaderStorageBlock(const std::string& name, uint32_t binding_point)
    {
        uint32_t idx = glGetProgramResourceIndex(m_id, GL_SHADER_STORAGE_BLOCK, name.c_str());
        if (idx != GL_INVALID_INDEX)
            glShaderStorageBlockBinding(m_id, idx, binding_point);
    }

} // namespace RealmEngine
