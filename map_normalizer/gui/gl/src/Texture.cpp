
#include "Texture.h"

#include <GL/glew.h>

#include "PreprocessorUtils.h"

std::queue<uint32_t> MapNormalizer::GUI::GL::Texture::available_tex_unit_ids;

ON_STATIC_INIT_BEGIN_BLOCK {
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE0);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE1);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE2);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE3);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE4);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE5);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE6);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE7);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE8);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE9);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE10);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE11);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE12);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE13);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE14);
    MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds().push(GL_TEXTURE15);
} ON_STATIC_INIT_END_BLOCK;

MapNormalizer::GUI::GL::Texture::TextureActivationFailure::TextureActivationFailure(const std::string& reason):
    m_reason(reason)
{ }

const char* MapNormalizer::GUI::GL::Texture::TextureActivationFailure::what() const noexcept
{
    return m_reason.c_str();
}

MapNormalizer::GUI::GL::Texture::Texture(): m_texture_id(-1) {
    glGenTextures(1, &m_texture_id);
}

MapNormalizer::GUI::GL::Texture::~Texture() {
    glDeleteTextures(1, &m_texture_id);
}

void MapNormalizer::GUI::GL::Texture::setWrapping(Axis axis,
                                                  WrapMode wrap)
{
    auto gl_target = targetToGLTarget(m_target);

    glBindTexture(gl_target, m_texture_id);
    glTexParameteri(gl_target, axisToGLAxis(axis), wrapToGLWrap(wrap));
}

void MapNormalizer::GUI::GL::Texture::setFiltering(FilterType ftype,
                                                   Filter filter)
{
    auto gl_target = targetToGLTarget(m_target);

    glBindTexture(gl_target, m_texture_id);
    glTexParameteri(gl_target, filterTypeToGLFilterType(ftype),
                    filterToGLFilter(filter));
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

    glBindTexture(gl_target, m_texture_id);
    glTexImage2D(gl_target, 0 /* mipmapping */,
                 gl_format, width, height, 0, gl_format, data_type, data);
}

/**
 * @brief Reserves a texture unit id if one is not already reserved
 */
void MapNormalizer::GUI::GL::Texture::activate() {
    if(m_texture_unit == INVALID_TEXTURE_UNIT_ID) {
        m_texture_unit = getNextAvailableTextureUnitID();
    }

    auto gl_target = targetToGLTarget(m_target);

    glActiveTexture(m_texture_unit);
    glBindTexture(gl_target, m_texture_id);
}

/**
 * @brief Frees up a texture unit
 */
void MapNormalizer::GUI::GL::Texture::deactivate() {
    freeTextureUnitID(m_texture_unit);
}

uint32_t MapNormalizer::GUI::GL::Texture::typeToDataType(const std::type_info& info)
{
    if(info == typeid(uint8_t*)) {
        return GL_UNSIGNED_BYTE;
    } else if(info == typeid(uint16_t*)) {
        return GL_UNSIGNED_SHORT;
    } else if(info == typeid(uint32_t*)) {
        return GL_UNSIGNED_INT;
    } else if(info == typeid(float*)) {
        return GL_FLOAT;
    } else if(info == typeid(double*)) {
        return GL_DOUBLE;
    } else {
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

uint32_t MapNormalizer::GUI::GL::Texture::filterToGLFilter(Filter filter) {
    return filter == Filter::NEAREST ? GL_NEAREST : GL_LINEAR;
}

uint32_t MapNormalizer::GUI::GL::Texture::filterTypeToGLFilterType(FilterType ftype)
{
    return ftype == FilterType::MAG ? GL_TEXTURE_MAG_FILTER : GL_TEXTURE_MIN_FILTER;
}

uint32_t MapNormalizer::GUI::GL::Texture::getNextAvailableTextureUnitID() {
    if(available_tex_unit_ids.empty()) {
        throw TextureActivationFailure("No more texture unit IDs are available.");
    } else {
        auto next_id = available_tex_unit_ids.front();
        available_tex_unit_ids.pop();

        return next_id;
    }
}

void MapNormalizer::GUI::GL::Texture::freeTextureUnitID(uint32_t& tex_unit_id) {
    if(tex_unit_id != INVALID_TEXTURE_UNIT_ID) {
        available_tex_unit_ids.push(tex_unit_id);
        tex_unit_id = INVALID_TEXTURE_UNIT_ID;
    }
}

std::queue<uint32_t> MapNormalizer::GUI::GL::Texture::getAvailableTexUnitIds() {
    return available_tex_unit_ids;
}

