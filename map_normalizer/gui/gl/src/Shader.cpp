#include "Shader.h"

#include <GL/glew.h>

MapNormalizer::GUI::GL::Shader::CompileException::CompileException(Type type,
                                                                   const std::string& reason):
    m_type(type),
    m_reason(reason)
{ }

const char* MapNormalizer::GUI::GL::Shader::CompileException::what() const noexcept
{
    return m_reason.c_str();
}

auto MapNormalizer::GUI::GL::Shader::CompileException::getType() const noexcept
    -> Type
{
    return m_type;
}


MapNormalizer::GUI::GL::Shader::Shader(Type type, const std::string& source):
    m_ref_count(new uint32_t{1}),
    m_shader_id(glCreateShader(typeToGL(type)))
{
    int status;
    glGetShaderiv(m_shader_id, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        int log_len;
        glGetShaderiv(m_shader_id, GL_INFO_LOG_LENGTH, &log_len);

        std::string info_log(log_len + 1, ' ');
        glGetShaderInfoLog(m_shader_id, log_len, nullptr, (GLchar*)info_log.c_str());

        glDeleteShader(m_shader_id);

        throw CompileException(type, info_log);
    }
}

MapNormalizer::GUI::GL::Shader::Shader(const Shader& other):
    m_ref_count(other.m_ref_count),
    m_shader_id(other.m_shader_id)
{
    (*m_ref_count)++;
}

MapNormalizer::GUI::GL::Shader::~Shader() {
    glDeleteShader(m_shader_id);
}

auto MapNormalizer::GUI::GL::Shader::operator=(const Shader& other) -> Shader& {
    m_ref_count = other.m_ref_count;
    m_shader_id = other.m_shader_id;

    (*m_ref_count)++;

    return *this;
}

uint32_t MapNormalizer::GUI::GL::Shader::getID() const {
    return m_shader_id;
}

uint32_t MapNormalizer::GUI::GL::Shader::typeToGL(Type type) {
    switch(type) {
        case Type::VERTEX:
            return GL_VERTEX_SHADER;
        case Type::FRAGMENT:
            return GL_FRAGMENT_SHADER;
        default:
            return -1;
    }
}

