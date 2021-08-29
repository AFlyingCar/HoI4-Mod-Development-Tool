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

MapNormalizer::GUI::GL::Texture::Texture(): m_texture_id(-1),
                                            m_texture_unit(-1),
                                            m_target(Target::TEX_2D)
{
    glGenTextures(1, &m_texture_id);
    MN_LOG_GL_ERRORS();
}

MapNormalizer::GUI::GL::Texture::~Texture() {
    glDeleteTextures(1, &m_texture_id);
    MN_LOG_GL_ERRORS();
}

/**
 * @brief Sets the wrapping mode for the given axis
 *
 * @param axis The axis to set
 * @param wrap The wrap mode to set
 */
void MapNormalizer::GUI::GL::Texture::setWrapping(Axis axis,
                                                  WrapMode wrap)
{
    auto gl_target = targetToGLTarget(m_target);

    bind();

    glTexParameteri(gl_target, axisToGLAxis(axis), wrapToGLWrap(wrap));
    MN_LOG_GL_ERRORS();
}

/**
 * @brief Sets the filter for the given filter type
 *
 * @details Implicitly calls bind()
 *
 * @param ftype The filter type
 * @param filter The filter
 */
void MapNormalizer::GUI::GL::Texture::setFiltering(FilterType ftype,
                                                   Filter filter)
{
    auto gl_target = targetToGLTarget(m_target);

    bind();

    glTexParameteri(gl_target, filterTypeToGLFilterType(ftype),
                    filterToGLFilter(filter));
    MN_LOG_GL_ERRORS();
}

void MapNormalizer::GUI::GL::Texture::setTextureUnitID(Unit unit) {
    m_texture_unit = unitToGLUnit(unit);
}

void MapNormalizer::GUI::GL::Texture::setTarget(Target target) {
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
void MapNormalizer::GUI::GL::Texture::setTextureData(Format format,
                                                     uint32_t width,
                                                     uint32_t height,
                                                     uint32_t data_type,
                                                     const void* data)
{
    auto gl_format = formatToGLFormat(format);
    auto gl_target = targetToGLTarget(m_target);

    bind();

    glTexImage2D(gl_target, 0 /* mipmapping */,
                 gl_format, width, height, 0, gl_format, data_type, data);
    MN_LOG_GL_ERRORS();
}

uint32_t MapNormalizer::GUI::GL::Texture::getTextureUnitID() {
    return m_texture_unit;
}

uint32_t MapNormalizer::GUI::GL::Texture::getTextureID() {
    return m_texture_id;
}

/**
 * @brief Binds or unbinds this texture
 *
 * @param do_bind Whether this texture should be bound or unbound.
 */
void MapNormalizer::GUI::GL::Texture::bind(bool do_bind) {
    auto gl_target = targetToGLTarget(m_target);

    if(do_bind) {
        glBindTexture(gl_target, m_texture_id);
    } else {
        glBindTexture(gl_target, 0);
    }
    MN_LOG_GL_ERRORS();
}

/**
 * @brief Activates this texture
 *
 * @details Implicitly calls bind()
 *
 * @return The texture unit that this texture is activated for
 */
uint32_t MapNormalizer::GUI::GL::Texture::activate() {
    glActiveTexture(m_texture_unit);
    MN_LOG_GL_ERRORS();

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
uint32_t MapNormalizer::GUI::GL::Texture::typeToDataType(const std::type_info& info)
{
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
uint32_t MapNormalizer::GUI::GL::Texture::targetToGLTarget(Target target) {
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
uint32_t MapNormalizer::GUI::GL::Texture::formatToGLFormat(Format format) {
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
uint32_t MapNormalizer::GUI::GL::Texture::axisToGLAxis(Axis axis) {
    return axis == Axis::S ? GL_TEXTURE_WRAP_S : GL_TEXTURE_WRAP_T;
}

/**
 * @brief Converts a WrapMode to the corresponding GL value
 *
 * @param target The WrapMode to convert
 *
 * @return The GL constant or -1 on error
 */
uint32_t MapNormalizer::GUI::GL::Texture::wrapToGLWrap(WrapMode wrap) {
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
uint32_t MapNormalizer::GUI::GL::Texture::unitToGLUnit(Unit unit) {
    return static_cast<uint32_t>(unit) + GL_TEXTURE0;
}

/**
 * @brief Converts a Filter to the corresponding GL value
 *
 * @param target The Filter to convert
 *
 * @return The GL constant.
 */
uint32_t MapNormalizer::GUI::GL::Texture::filterToGLFilter(Filter filter) {
    return filter == Filter::NEAREST ? GL_NEAREST : GL_LINEAR;
}

/**
 * @brief Converts a FilterType to the corresponding GL value
 *
 * @param target The FilterType to convert
 *
 * @return The GL constant.
 */
uint32_t MapNormalizer::GUI::GL::Texture::filterTypeToGLFilterType(FilterType ftype)
{
    return ftype == FilterType::MAG ? GL_TEXTURE_MAG_FILTER : GL_TEXTURE_MIN_FILTER;
}

