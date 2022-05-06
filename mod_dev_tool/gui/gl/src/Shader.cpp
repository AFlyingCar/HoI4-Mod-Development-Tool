/**
 * @file Shader.cpp
 *
 * @brief Defines the Shader class
 */

#include "Shader.h"

#include <sstream>

#include <GL/glew.h>

#include "GLUtils.h"

#include "Util.h"

std::map<std::string, std::string> HMDT::GUI::GL::Shader::m_defined_macros;

HMDT::GUI::GL::Shader::CompileException::CompileException(Type type,
                                                          const std::string& source,
                                                          const std::string& reason):
    m_type(type),
    m_source(source),
    m_reason(reason)
{
    // Properly construct the 'what' message
    std::stringstream what;
    what << std::to_string(m_type) << " failure: " << m_reason << "\n```"
         << m_source << "\n```";
    m_what = what.str();
}

const char* HMDT::GUI::GL::Shader::CompileException::what() const noexcept {
    return m_what.c_str();
}

auto HMDT::GUI::GL::Shader::CompileException::getType() const noexcept -> Type {
    return m_type;
}

/**
 * @brief Constructs and compiles a single Shader
 *
 * @param type The Shader type being compiled
 * @param source The source code to compile
 */
HMDT::GUI::GL::Shader::Shader(Type type, const std::string& source):
    m_ref_count(new uint32_t{1}), m_shader_id(-1)
{
    // Create the shader
    m_shader_id = glCreateShader(typeToGL(type));

    // Compile the shader
    int status;
    //   Transform the source-code with any defined macros first
    auto new_source = addMacroDefinitions(source);
    auto* source_cstr = new_source.c_str();
    GLint source_len = new_source.size();
    glShaderSource(m_shader_id, 1, &(source_cstr), &source_len);
    glCompileShader(m_shader_id);

    // Check for compilation errors
    glGetShaderiv(m_shader_id, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        int log_len;
        glGetShaderiv(m_shader_id, GL_INFO_LOG_LENGTH, &log_len);

        std::string info_log(log_len + 1, ' ');
        glGetShaderInfoLog(m_shader_id, log_len, nullptr, (GLchar*)info_log.c_str());

        glDeleteShader(m_shader_id);

        throw CompileException(type, source, info_log);
    }
}

HMDT::GUI::GL::Shader::Shader(const Shader& other):
    m_ref_count(other.m_ref_count),
    m_shader_id(other.m_shader_id)
{
    (*m_ref_count)++;
}

HMDT::GUI::GL::Shader::~Shader() {
    // If there is no reference count defined, then do not delete
    if(m_ref_count == nullptr) return;

    --(*m_ref_count);

    if(*m_ref_count <= 0) {
        glDeleteShader(m_shader_id);
        HMDT_LOG_GL_ERRORS();
    }
}

auto HMDT::GUI::GL::Shader::operator=(const Shader& other) -> Shader& {
    m_ref_count = other.m_ref_count;
    m_shader_id = other.m_shader_id;

    (*m_ref_count)++;

    return *this;
}

uint32_t HMDT::GUI::GL::Shader::getID() const {
    return m_shader_id;
}

/**
 * @brief Converts a Shader::Type into its OpenGL representation
 *
 * @param type The type to convert.
 *
 * @return The OpenGL constant relating to the given Shader::Type
 */
uint32_t HMDT::GUI::GL::Shader::typeToGL(Type type) {
    switch(type) {
        case Type::VERTEX:
            return GL_VERTEX_SHADER;
        case Type::FRAGMENT:
            return GL_FRAGMENT_SHADER;
        default:
            return -1;
    }
}

/**
 * @brief Converts a Shader::Type to a string
 *
 * @param type The type to convert
 *
 * @return A string representation of the given Shader::Type
 */
std::string_view std::to_string(const HMDT::GUI::GL::Shader::Type& type) {
    switch(type) {
        case HMDT::GUI::GL::Shader::Type::VERTEX:
            return "Vertex";
        case HMDT::GUI::GL::Shader::Type::FRAGMENT:
            return "Fragment";
        default:
            return "<INVALID>";
    }
}

void HMDT::GUI::GL::Shader::undefineMacro(const std::string& macro_name) {
    m_defined_macros.erase(macro_name);
}

std::string HMDT::GUI::GL::Shader::addMacroDefinitions(const std::string& source)
{
    std::string src_to_version;
    std::string src_post_version = source;
    if(auto version_idx = source.find("#version");
            version_idx != std::string::npos)
    {
        // All code up to the end of the #version line
        auto nl = source.find("\n", version_idx);
        src_to_version = source.substr(0, nl + 1);
        src_post_version = source.substr(nl + 1);
    }

    // Build the source
    std::stringstream src;
    src << src_to_version << std::endl;
    for(auto&& [k, v] : m_defined_macros) {
        src << "#define " << k << ' ' << v << std::endl;
    }
    src << "\n" << src_post_version;

    return src.str();
}

