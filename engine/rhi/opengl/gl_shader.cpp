#include "rhi/opengl/gl_shader.h"

#include <fstream>
#include <sstream>

#include <glad/gl.h>
#include "core/log/log_macros.h"

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
            return ss.str();
        }
        catch (const std::ifstream::failure&)
        {
            RE_LOG_ERROR("Failed to read shader file: " + path);
            return {};
        }
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

    GLShader::~GLShader()
    {
        if (m_id != 0)
            glDeleteProgram(m_id);
    }

    // ----- interface ------------------------------------------------------

    void GLShader::use() { glUseProgram(m_id); }

    void GLShader::setBool(const std::string& name, bool value)
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
    }

    void GLShader::setInt(const std::string& name, int value)
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void GLShader::setFloat(const std::string& name, float value)
    {
        glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void GLShader::setVec2(const std::string& name, const glm::vec2& value)
    {
        glUniform2f(glGetUniformLocation(m_id, name.c_str()), value.x, value.y);
    }

    void GLShader::setVec3(const std::string& name, const glm::vec3& value)
    {
        glUniform3f(glGetUniformLocation(m_id, name.c_str()), value.x, value.y, value.z);
    }

    void GLShader::setMat4(const std::string& name, const glm::mat4& value)
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &value[0][0]);
    }

    void GLShader::setVec3Array(const std::string& name, const std::vector<glm::vec3>& values)
    {
        glUniform3fv(glGetUniformLocation(m_id, name.c_str()), static_cast<GLsizei>(values.size()), &values[0][0]);
    }

    void GLShader::setFloatArray(const std::string& name, const std::vector<float>& values)
    {
        glUniform1fv(glGetUniformLocation(m_id, name.c_str()), static_cast<GLsizei>(values.size()), values.data());
    }

    void GLShader::setIntArray(const std::string& name, const std::vector<int>& values)
    {
        glUniform1iv(glGetUniformLocation(m_id, name.c_str()), static_cast<GLsizei>(values.size()), values.data());
    }

    void GLShader::setMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
    {
        setMat4("model", model);
        setMat4("view", view);
        setMat4("projection", projection);
    }

    void GLShader::bindUniformBlock(const std::string& name, uint32_t binding_point)
    {
        uint32_t idx = glGetUniformBlockIndex(m_id, name.c_str());
        if (idx != GL_INVALID_INDEX)
            glUniformBlockBinding(m_id, idx, binding_point);
    }

} // namespace RealmEngine
