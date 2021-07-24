
#include "Program.h"

#include <GL/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Logger.h"

MapNormalizer::GUI::GL::Program::LinkException::LinkException(const std::string& reason):
    m_reason(reason)
{ }

const char* MapNormalizer::GUI::GL::Program::LinkException::what() const noexcept
{
    return (std::string("Program Link Failure: ") + m_reason).c_str();
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

/**
 * @brief Sets an OpenGL uniform
 * @details Can throw a std::bad_any_cast
 *
 * @param uniform_name The name of the uniform to set
 * @param value The value to set
 */
void MapNormalizer::GUI::GL::Program::uniform(const std::string& uniform_name,
                                              const std::any& value)
{
    auto uniform_loc = glGetUniformLocation(m_program_id, uniform_name.c_str());

    if(value.type() == typeid(bool)) {                                 // bool
        glUniform1i(uniform_loc, std::any_cast<bool>(value));
    } else if(value.type() == typeid(int)) {                           // int
        glUniform1i(uniform_loc, std::any_cast<int>(value));
    } else if(value.type() == typeid(float)) {                         // float
        glUniform1f(uniform_loc, std::any_cast<float>(value));
    } else if(value.type() == typeid(glm::vec2)) {                     // vec2
        glUniform2fv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::vec2>(value)));
    } else if(value.type() == typeid(glm::vec3)) {                     // vec3
        glUniform3fv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::vec3>(value)));
    } else if(value.type() == typeid(glm::vec4)) {                     // vec4
        glUniform4fv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::vec4>(value)));
    } else if(value.type() == typeid(glm::mat2)) {                     // mat2
        glUniformMatrix2fv(uniform_loc, 1, GL_FALSE,
                           glm::value_ptr(std::any_cast<glm::mat2>(value)));
    } else if(value.type() == typeid(glm::mat3)) {                     // mat3
        glUniformMatrix3fv(uniform_loc, 1, GL_FALSE,
                           glm::value_ptr(std::any_cast<glm::mat3>(value)));
    } else if(value.type() == typeid(glm::mat4)) {                     // mat4
        glUniformMatrix4fv(uniform_loc, 1, GL_FALSE,
                           glm::value_ptr(std::any_cast<glm::mat4>(value)));
    } else {
        writeError<true>("Unsupported type ", value.type().name());
        // TODO: Should we return a value on error? throw?
    }
}

