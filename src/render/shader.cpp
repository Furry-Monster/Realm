#include "render/shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "utils.h"
#include <glad/gl.h>

namespace RealmEngine
{
    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        // load shaders
        std::string   vertex_code;
        std::string   fragment_code;
        std::ifstream vertex_shader_file;
        std::ifstream fragment_shader_file;

        vertex_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fragment_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vertex_shader_file.open(vertexPath);
            fragment_shader_file.open(fragmentPath);

            std::stringstream vertex_shader_stream, fragment_shader_stream;

            vertex_shader_stream << vertex_shader_file.rdbuf();
            fragment_shader_stream << fragment_shader_file.rdbuf();

            vertex_shader_file.close();
            fragment_shader_file.close();

            vertex_code   = vertex_shader_stream.str();
            fragment_code = fragment_shader_stream.str();
        }
        catch (std::ifstream::failure& e)
        {
            err("Error: failed to read shader file: " + vertexPath + " or " + fragmentPath);
            m_id = 0;
            return;
        }

        const char* vertex_shader_code   = vertex_code.c_str();
        const char* fragment_shader_code = fragment_code.c_str();

        // compile shaders
        unsigned int vertex, fragment;
        int          success;
        char         info_log[512];

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_shader_code, nullptr);
        glCompileShader(vertex);

        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, nullptr, info_log);
            err("Error: vertex shader compilation failed for: " + vertexPath);
            err(std::string(info_log));
            m_id = 0;
            return;
        }

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragment_shader_code, nullptr);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, nullptr, info_log);
            err("Error: fragment shader compilation failed for: " + fragmentPath);
            err(std::string(info_log));
            glDeleteShader(vertex);
            m_id = 0;
            return;
        }

        // link shaders
        m_id = glCreateProgram();
        glAttachShader(m_id, vertex);
        glAttachShader(m_id, fragment);
        glLinkProgram(m_id);

        glGetProgramiv(m_id, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(m_id, 512, nullptr, info_log);
            err("Error: shader program linking failed");
            err(vertexPath);
            err(fragmentPath);
            err(std::string(info_log));
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            m_id = 0;
            return;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    Shader::~Shader() noexcept
    {
        if (m_id != 0)
            glDeleteProgram(m_id);
    }

    void Shader::use() const { glUseProgram(m_id); }

    void Shader::setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
    }

    void Shader::setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void Shader::setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void Shader::setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2f(glGetUniformLocation(m_id, name.c_str()), value[0], value[1]);
    }

    void Shader::setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3f(glGetUniformLocation(m_id, name.c_str()), value[0], value[1], value[2]);
    }

    void Shader::setVec3Array(const std::string& name, const std::vector<glm::vec3>& values) const
    {
        glUniform3fv(glGetUniformLocation(m_id, name.c_str()), values.size(), &values[0][0]);
    }

    void Shader::setMat4(const std::string& name, const glm::mat4& value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::setModelViewProjectionMatrices(const glm::mat4& model,
                                                const glm::mat4& view,
                                                const glm::mat4& projection) const
    {
        setMat4("model", model);
        setMat4("view", view);
        setMat4("projection", projection);
    }
} // namespace RealmEngine
