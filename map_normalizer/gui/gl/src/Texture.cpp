
#include "Texture.h"

#include <GL/glew.h>

#include "Logger.h"
#include "PreprocessorUtils.h"

#include "GLUtils.h"

MapNormalizer::GUI::GL::Texture::TextureActivationFailure::TextureActivationFailure(const std::string& reason):
    m_reason(reason)
{ }

const char* MapNormalizer::GUI::GL::Texture::TextureActivationFailure::what() const noexcept
{
    return m_reason.c_str();
}

MapNormalizer::GUI::GL::Texture::Texture(): m_texture_id(-1),
                                            m_texture_unit(-1),
                                            m_target(Target::TEX_2D)
{
    writeDebug<true>("glGenTextures(1, ", &m_texture_id, ')');
    glGenTextures(1, &m_texture_id);
    MN_LOG_GL_ERRORS();
}

MapNormalizer::GUI::GL::Texture::~Texture() {
    glDeleteTextures(1, &m_texture_id);
    MN_LOG_GL_ERRORS();
}

void MapNormalizer::GUI::GL::Texture::setWrapping(Axis axis,
                                                  WrapMode wrap)
{
    auto gl_target = targetToGLTarget(m_target);

    bind();

    glTexParameteri(gl_target, axisToGLAxis(axis), wrapToGLWrap(wrap));
    MN_LOG_GL_ERRORS();
}

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

void MapNormalizer::GUI::GL::Texture::setTextureData(Format format,
                                                     uint32_t width,
                                                     uint32_t height,
                                                     uint32_t data_type,
                                                     const void* data)
{
    auto gl_format = formatToGLFormat(format);
    auto gl_target = targetToGLTarget(m_target);

    bind();

    writeDebug<true>("glTexImage2D(", glEnumToStrings(gl_target).front(), ',', 0 /* mipmapping */, ',',
                     glEnumToStrings(gl_format).front(), ',', width, ',', height, ',', 0, ',', glEnumToStrings(gl_format).front(),
                     ',', glEnumToStrings(data_type).front(), ',', data, ')');
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

void MapNormalizer::GUI::GL::Texture::bind(bool do_bind) {
    auto gl_target = targetToGLTarget(m_target);

    if(do_bind) {
        writeDebug<true>("glBindTexture(", glEnumToStrings(gl_target).front(), ',', m_texture_id, ')');
        glBindTexture(gl_target, m_texture_id);
    } else {
        writeDebug<true>("glBindTexture(", glEnumToStrings(gl_target).front(), ",0)");
        glBindTexture(gl_target, 0);
    }
    MN_LOG_GL_ERRORS();
}

uint32_t MapNormalizer::GUI::GL::Texture::activate() {
    writeDebug<true>("glActiveTexture(", glEnumToStrings(m_texture_unit).front(), ')');
    glActiveTexture(m_texture_unit);
    MN_LOG_GL_ERRORS();

    bind();

    return m_texture_unit;
}

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
        writeError<true>("Invalid data type given: ", info.name());
        return -1;
    }
}

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

uint32_t MapNormalizer::GUI::GL::Texture::formatToGLFormat(Format format) {
    switch(format) {
        case Format::RGB:
            return GL_RGB;
        default:
            return -1;
    }
}

uint32_t MapNormalizer::GUI::GL::Texture::axisToGLAxis(Axis axis) {
    return axis == Axis::S ? GL_TEXTURE_WRAP_S : GL_TEXTURE_WRAP_T;
}

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

uint32_t MapNormalizer::GUI::GL::Texture::unitToGLUnit(Unit unit) {
    return static_cast<uint32_t>(unit) + GL_TEXTURE0;
}

uint32_t MapNormalizer::GUI::GL::Texture::filterToGLFilter(Filter filter) {
    return filter == Filter::NEAREST ? GL_NEAREST : GL_LINEAR;
}

uint32_t MapNormalizer::GUI::GL::Texture::filterTypeToGLFilterType(FilterType ftype)
{
    return ftype == FilterType::MAG ? GL_TEXTURE_MAG_FILTER : GL_TEXTURE_MIN_FILTER;
}

