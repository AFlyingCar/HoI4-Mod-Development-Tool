/**
 * @file Texture.cpp
 *
 * @brief Defines the Texture class
 */

#include "Texture.h"

#include <GL/glew.h>

#include "Logger.h"
#include "PreprocessorUtils.h"

#include "GLUtils.h"

HMDT::GUI::GL::Texture::Texture(): m_texture_id(-1), m_texture_unit(-1),
                                   m_target(Target::TEX_2D)
{
    glGenTextures(1, &m_texture_id);
    HMDT_LOG_GL_ERRORS();
}

HMDT::GUI::GL::Texture::~Texture() {
    glDeleteTextures(1, &m_texture_id);
    HMDT_LOG_GL_ERRORS();
}

/**
 * @brief Sets the wrapping mode for the given axis
 *
 * @param axis The axis to set
 * @param wrap The wrap mode to set
 */
void HMDT::GUI::GL::Texture::setWrapping(Axis axis, WrapMode wrap) {
    auto gl_target = targetToGLTarget(m_target);

    bind();

    glTexParameteri(gl_target, axisToGLAxis(axis), wrapToGLWrap(wrap));
    HMDT_LOG_GL_ERRORS();
}

/**
 * @brief Sets the filter for the given filter type
 *
 * @details Implicitly calls bind()
 *
 * @param ftype The filter type
 * @param filter The filter
 */
void HMDT::GUI::GL::Texture::setFiltering(FilterType ftype, Filter filter) {
    auto gl_target = targetToGLTarget(m_target);

    bind();

    glTexParameteri(gl_target, filterTypeToGLFilterType(ftype),
                    filterToGLFilter(filter));
    HMDT_LOG_GL_ERRORS();
}

void HMDT::GUI::GL::Texture::setTextureUnitID(Unit unit) {
    m_texture_unit = unitToGLUnit(unit);
}

void HMDT::GUI::GL::Texture::setTarget(Target target) {
    m_target = target;
}

/**
 * @brief Sends texture data to the GPU.
 *
 * @details Implicitly calls bind()
 *
 * @param format The format of the data. Currently, both the CPU-side and
 *               GPU-side format must match.
 * @param width The width of the data
 * @param height The height of the data
 * @param data_type The data type being passed in
 * @param data The data to send to the GPU
 */
void HMDT::GUI::GL::Texture::setTextureData(Format internal_format,
                                            uint32_t width, uint32_t height,
                                            uint32_t data_type,
                                            const void* data,
                                            std::optional<uint32_t> format)
{
    auto gl_int_format = formatToGLFormat(internal_format);
    auto gl_target = targetToGLTarget(m_target);

    uint32_t gl_format = gl_int_format;
    if(format) {
        gl_format = *format;
    }

    bind();

    WRITE_DEBUG("Creating (", width, "x", height, ") texture with parameters: ",
                std::hex,
                "TARGET=", gl_target, ", INT_FORMAT=", gl_int_format,
                ", FORMAT=", gl_format, ", DATA_TYPE=", data_type, ", DATA=0x",
                data, std::dec);

    glTexImage2D(gl_target, 0 /* mipmapping */,
                 gl_int_format, width, height, 0, gl_format, data_type, data);
    HMDT_LOG_GL_ERRORS();

    m_width = width;
    m_height = height;
}

uint32_t HMDT::GUI::GL::Texture::getTextureUnitID() const {
    return m_texture_unit;
}

uint32_t HMDT::GUI::GL::Texture::getTextureID() const {
    return m_texture_id;
}

uint32_t HMDT::GUI::GL::Texture::getWidth() const {
    return m_width;
}

uint32_t HMDT::GUI::GL::Texture::getHeight() const {
    return m_height;
}

std::pair<uint32_t, uint32_t> HMDT::GUI::GL::Texture::getDimensions() const {
    return std::make_pair(m_width, m_height);
}

/**
 * @brief Binds or unbinds this texture
 *
 * @param do_bind Whether this texture should be bound or unbound.
 */
void HMDT::GUI::GL::Texture::bind(bool do_bind) {
    auto gl_target = targetToGLTarget(m_target);

    if(do_bind) {
        glBindTexture(gl_target, m_texture_id);
    } else {
        glBindTexture(gl_target, 0);
    }
    HMDT_LOG_GL_ERRORS();
}

/**
 * @brief Activates this texture
 *
 * @details Implicitly calls bind()
 *
 * @return The texture unit that this texture is activated for
 */
uint32_t HMDT::GUI::GL::Texture::activate() {
    glActiveTexture(m_texture_unit);
    HMDT_LOG_GL_ERRORS();

    bind();

    return m_texture_unit;
}

/**
 * @brief Converts a std::type_info to the corresponding GL type
 *
 * @param info The std::type_info to convert.
 *
 * @return The GL type constant, or -1 on error.
 */
uint32_t HMDT::GUI::GL::Texture::typeToDataType(const std::type_info& info) {
    if(info == typeid(uint8_t) || info == typeid(unsigned char)) {
        return GL_UNSIGNED_BYTE;
    } else if(info == typeid(uint16_t)) {
        return GL_UNSIGNED_SHORT;
    } else if(info == typeid(uint32_t)) {
        return GL_UNSIGNED_INT;
    } else if(info == typeid(float)) {
        return GL_FLOAT;
    } else if(info == typeid(double)) {
        return GL_DOUBLE;
    } else {
        WRITE_ERROR("Invalid data type given: ", info.name());
        return -1;
    }
}

/**
 * @brief Converts a Target to the corresponding GL value
 *
 * @param target The Target to convert
 *
 * @return The GL constant or -1 on error
 */
uint32_t HMDT::GUI::GL::Texture::targetToGLTarget(Target target) {
    switch(target) {
        case Target::TEX_1D:
            return GL_TEXTURE_1D;
        case Target::TEX_2D:
            return GL_TEXTURE_2D;
        default:
            return -1;
    }
}

/**
 * @brief Converts a Format to the corresponding GL value
 *
 * @param target The Target to convert
 *
 * @return The GL constant or -1 on error
 */
uint32_t HMDT::GUI::GL::Texture::formatToGLFormat(Format format) {
    switch(format) {
        case Format::RED:
            return GL_RED;
        case Format::GREEN:
            return GL_GREEN;
        case Format::BLUE:
            return GL_BLUE;
        case Format::ALPHA:
            return GL_ALPHA;
        case Format::RGB:
            return GL_RGB;
        case Format::RGBA:
            return GL_RGBA;
        case Format::RED32I:
            return GL_R32I;
        case Format::RED32UI:
            return GL_R32UI;
        default:
            return -1;
    }
}

/**
 * @brief Converts a Axis to the corresponding GL value
 *
 * @param target The Axis to convert
 *
 * @return The GL constant or -1 on error
 */
uint32_t HMDT::GUI::GL::Texture::axisToGLAxis(Axis axis) {
    return axis == Axis::S ? GL_TEXTURE_WRAP_S : GL_TEXTURE_WRAP_T;
}

/**
 * @brief Converts a WrapMode to the corresponding GL value
 *
 * @param target The WrapMode to convert
 *
 * @return The GL constant or -1 on error
 */
uint32_t HMDT::GUI::GL::Texture::wrapToGLWrap(WrapMode wrap) {
    switch(wrap) {
        case WrapMode::REPEAT:
            return GL_REPEAT;
        case WrapMode::MIRRORED_REPEAT:
            return GL_MIRRORED_REPEAT;
        case WrapMode::CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        case WrapMode::CLAMP_TO_BORDER:
            return GL_CLAMP_TO_BORDER;
        default:
            return -1;
    }
}

/**
 * @brief Converts a Unit to the corresponding GL value
 *
 * @param target The Unit to convert
 *
 * @return The GL constant.
 */
uint32_t HMDT::GUI::GL::Texture::unitToGLUnit(Unit unit) {
    return static_cast<uint32_t>(unit) + GL_TEXTURE0;
}

/**
 * @brief Converts a Filter to the corresponding GL value
 *
 * @param target The Filter to convert
 *
 * @return The GL constant.
 */
uint32_t HMDT::GUI::GL::Texture::filterToGLFilter(Filter filter) {
    return filter == Filter::NEAREST ? GL_NEAREST : GL_LINEAR;
}

/**
 * @brief Converts a FilterType to the corresponding GL value
 *
 * @param target The FilterType to convert
 *
 * @return The GL constant.
 */
uint32_t HMDT::GUI::GL::Texture::filterTypeToGLFilterType(FilterType ftype) {
    return ftype == FilterType::MAG ? GL_TEXTURE_MAG_FILTER : GL_TEXTURE_MIN_FILTER;
}

