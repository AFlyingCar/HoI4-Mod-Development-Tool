
#include "Program.h"

#include <GL/glew.h>

MapNormalizer::GUI::GL::Program::LinkException::LinkException(const std::string& reason):
    m_reason(reason)
{ }

const char* MapNormalizer::GUI::GL::Program::LinkException::what() const noexcept
{
    return m_reason.c_str();
}

MapNormalizer::GUI::GL::Program::Program(): m_ref_count(nullptr),
                                            m_program_id(-1)
{ }

MapNormalizer::GUI::GL::Program::Program(std::initializer_list<Shader> shaders):
    m_ref_count(new uint32_t{1}),
    m_program_id(glCreateProgram())
{
    attachShaders(shaders.begin(), shaders.end());
    linkProgram();
}

MapNormalizer::GUI::GL::Program::Program(const std::vector<Shader>& shaders):
    m_ref_count(new uint32_t{1}),
    m_program_id(glCreateProgram())
{
    attachShaders(shaders.begin(), shaders.end());
    linkProgram();
}

MapNormalizer::GUI::GL::Program::Program(const Program& other):
    m_ref_count(other.m_ref_count),
    m_program_id(other.m_program_id)
{
    ++(*m_ref_count);
}

MapNormalizer::GUI::GL::Program::~Program() {
    // If there is no reference count defined, then do not delete
    if(m_ref_count == nullptr) return;

    --(*m_ref_count);

    if(*m_ref_count <= 0) {
        glDeleteProgram(m_program_id);
    }
}

auto MapNormalizer::GUI::GL::Program::operator=(const Program& other) -> Program& {
    m_ref_count = other.m_ref_count;
    m_program_id = other.m_program_id;

    ++(*m_ref_count);

    return *this;
}

/**
 * @brief Use this program, or stop using it
 *
 * @param load Whether the program should be loaded or unloaded
 */
void MapNormalizer::GUI::GL::Program::use(bool load) {
    if(load) {
        glUseProgram(m_program_id);
    } else {
        glUseProgram(0);
    }
}

void MapNormalizer::GUI::GL::Program::attachShader(const Shader& shader) {
    glAttachShader(m_program_id, shader.getID());
}

void MapNormalizer::GUI::GL::Program::linkProgram() {
    glLinkProgram(m_program_id);

    int status;
    glGetProgramiv(m_program_id, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        int log_len;
        glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &log_len);

        std::string info_log(log_len + 1, ' ');
        glGetProgramInfoLog(m_program_id, log_len, nullptr, (GLchar*)info_log.c_str());

        glDeleteProgram(m_program_id);

        throw LinkException(info_log);
    }
}

