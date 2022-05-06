/**
 * @file Program.cpp
 *
 * @brief Defines the Program class
 */

#include "Program.h"

#include <GL/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Types.h"

#include "Logger.h"

#include "GLUtils.h"
#include "Texture.h"

HMDT::GUI::GL::Program::LinkException::LinkException(const std::string& reason):
    m_reason(reason)
{ }

const char* HMDT::GUI::GL::Program::LinkException::what() const noexcept {
    return (std::string("Program Link Failure: ") + m_reason).c_str();
}

HMDT::GUI::GL::Program::Program(): m_ref_count(nullptr), m_program_id(-1) { }

HMDT::GUI::GL::Program::Program(std::initializer_list<Shader> shaders):
    m_ref_count(new uint32_t{1}),
    m_program_id(glCreateProgram())
{
    attachShaders(shaders.begin(), shaders.end());
    linkProgram();
}

HMDT::GUI::GL::Program::Program(const std::vector<Shader>& shaders):
    m_ref_count(new uint32_t{1}),
    m_program_id(glCreateProgram())
{
    attachShaders(shaders.begin(), shaders.end());
    linkProgram();
}

HMDT::GUI::GL::Program::Program(const Program& other):
    m_ref_count(other.m_ref_count),
    m_program_id(other.m_program_id)
{
    ++(*m_ref_count);
}

HMDT::GUI::GL::Program::~Program() {
    // If there is no reference count defined, then do not delete
    if(m_ref_count == nullptr) return;

    --(*m_ref_count);

    if(*m_ref_count <= 0) {
        glDeleteProgram(m_program_id);
        MN_LOG_GL_ERRORS();
    }
}

auto HMDT::GUI::GL::Program::operator=(const Program& other) -> Program& {
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
void HMDT::GUI::GL::Program::use(bool load) {
    if(load) {
        glUseProgram(m_program_id);
    } else {
        glUseProgram(0);
    }
    MN_LOG_GL_ERRORS();
}

/**
 * @brief Attaches a single Shader to this program
 *
 * @param shader The Shader to attach
 */
void HMDT::GUI::GL::Program::attachShader(const Shader& shader) {
    glAttachShader(m_program_id, shader.getID());
    MN_LOG_GL_ERRORS();
}

/**
 * @brief Links all attached shaders together
 *
 * @throw LinkException Will throw if GL_LINK_STATUS is not GL_TRUE after linking
 */
void HMDT::GUI::GL::Program::linkProgram() {
    glLinkProgram(m_program_id);
    MN_LOG_GL_ERRORS();

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
 *
 * @throw std::bad_any_cast 
 *
 * @param uniform_name The name of the uniform to set
 * @param value The value to set
 *
 * @return true on success, false otherwise
 */
bool HMDT::GUI::GL::Program::uniform(const std::string& uniform_name,
                                     const std::any& value)
{
    use();

    auto uniform_loc = glGetUniformLocation(m_program_id, uniform_name.c_str());

    if(value.type() == typeid(bool)) {                                 // bool
        // NOTE: No support at the moment for bool[] due to issues with std::vector<bool>
        glUniform1i(uniform_loc, std::any_cast<bool>(value));
    } else if(value.type() == typeid(int)) {                           // int
        glUniform1i(uniform_loc, std::any_cast<int>(value));
    } else if(value.type() == typeid(std::vector<int>)) {              // int[]
        auto vec = std::any_cast<std::vector<int>>(value);
        glUniform1iv(uniform_loc, vec.size(), vec.data());
    } else if(value.type() == typeid(uint32_t)) {                      // unsigned int
        glUniform1ui(uniform_loc, std::any_cast<uint32_t>(value));
    } else if(value.type() == typeid(std::vector<unsigned int>)) {     // unsigned int[]
        auto vec = std::any_cast<std::vector<unsigned int>>(value);
        glUniform1uiv(uniform_loc, vec.size(), vec.data());
    } else if(value.type() == typeid(float)) {                         // float
        glUniform1f(uniform_loc, std::any_cast<float>(value));
    } else if(value.type() == typeid(std::vector<float>)) {            // float[]
        auto vec = std::any_cast<std::vector<float>>(value);
        glUniform1fv(uniform_loc, vec.size(), vec.data());
    } else if(value.type() == typeid(glm::vec2)) {                     // vec2
        glUniform2fv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::vec2>(value)));
    } else if(value.type() == typeid(glm::ivec2)) {                    // ivec2
        glUniform2iv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::ivec2>(value)));
    } else if(value.type() == typeid(glm::vec3)) {                     // vec3
        glUniform3fv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::vec3>(value)));
    } else if(value.type() == typeid(glm::ivec3)) {                    // ivec3
        glUniform3iv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::ivec3>(value)));
    } else if(value.type() == typeid(Color)) {                         // Color
        auto&& color = std::any_cast<Color>(value);
        glUniform3f(uniform_loc, color.r / 255.0f,
                                 color.g / 255.0f,
                                 color.b / 255.0f);
    } else if(value.type() == typeid(glm::vec4)) {                     // vec4
        glUniform4fv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::vec4>(value)));
    } else if(value.type() == typeid(glm::ivec4)) {                    // ivec4
        glUniform4iv(uniform_loc, 1,
                     glm::value_ptr(std::any_cast<glm::ivec4>(value)));
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
        WRITE_ERROR("Unsupported type ", value.type().name());
        return false;
    }

    if(MN_LOG_GL_ERRORS() >= 1) {
        WRITE_ERROR("Failed to set uniform ", uniform_name);
        return false;
    }

    return true;
}

bool HMDT::GUI::GL::Program::uniform(const std::string& uniform_name,
                                     const Texture& value)
{
    return uniform(uniform_name, static_cast<int>(value.getTextureUnitID()) - GL_TEXTURE0);
}

