/**
 * @file MapRenderingViewBase.h
 *
 * @brief A base class for rendering just the map by itself.
 */

#include "MapRenderingViewBase.h"

#include <GL/glew.h>

#include "Logger.h"

#include "GLUtils.h"
#include "GLShaderSources.h"

MapNormalizer::GUI::GL::MapRenderingViewBase::MapRenderingViewBase() {
}

MapNormalizer::GUI::GL::MapRenderingViewBase::~MapRenderingViewBase() {
    if(m_vao != -1) {
        glDeleteVertexArrays(1, &m_vao);
        MN_LOG_GL_ERRORS();
    }
}

/**
 * @brief Initializes this MapRenderingViewBase
 *
 * @details Will build the basic shader program as well as build the geometry
 *          that will be used for drawing the map
 */
void MapNormalizer::GUI::GL::MapRenderingViewBase::init() {
    // Set up all shaders
    WRITE_DEBUG("Building map rendering view program...");
    m_program = Program{Shader(Shader::Type::VERTEX,
                               ShaderSources::provinceview_vertex),
                        Shader(Shader::Type::FRAGMENT,
                               ShaderSources::provinceview_fragment)
                       };
    MN_LOG_GL_ERRORS();

    WRITE_DEBUG("Building map object...");
    {
        // NOTE: We could use EBOs here, but we are only going to have this one
        //   object, so there really isn't much of a point

        // gen vertex arrays
        glGenVertexArrays(1, &m_vao);
        MN_LOG_GL_ERRORS();

        // gen buffers
        glGenBuffers(1, &m_vbo);
        MN_LOG_GL_ERRORS();

        // bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        MN_LOG_GL_ERRORS();

        // buffer data
        auto vertices = getMapVertices();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(*vertices.data()),
                     vertices.data(), GL_STATIC_DRAW);
        MN_LOG_GL_ERRORS();

        // bind vertex arrays
        glBindVertexArray(m_vao);
        MN_LOG_GL_ERRORS();

        glEnableVertexAttribArray(0); // Location 0
        MN_LOG_GL_ERRORS();
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        MN_LOG_GL_ERRORS();

        // unbind buffer and vertex array
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        MN_LOG_GL_ERRORS();
        glBindVertexArray(0);
        MN_LOG_GL_ERRORS();
    }
}

/**
 * @brief Re-generates the map texture based on the data stored in map_data
 */
void MapNormalizer::GUI::GL::MapRenderingViewBase::onMapDataChanged(std::shared_ptr<const MapData> map_data)
{
    auto [iwidth, iheight] = map_data->getDimensions();

    m_texture.setTextureUnitID(Texture::Unit::TEX_UNIT0);

    m_texture.bind();
    {
        // map_texture->setWrapping(Texture::Axis::S, Texture::WrapMode::CLAMP_TO_EDGE);
        // map_texture->setWrapping(Texture::Axis::T, Texture::WrapMode::CLAMP_TO_EDGE);

        m_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
        m_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

        m_texture.setTextureData(Texture::Format::RGB,
                                  iwidth, iheight, map_data->getProvinces().lock().get());
    }
    m_texture.bind(false);

    ////////////////////////////////////////////////////////////////////////////

    m_label_texture.setTextureUnitID(Texture::Unit::TEX_UNIT1);

    m_label_texture.bind();
    {
        WRITE_DEBUG("Building label matrix texture.");
        m_label_texture.setWrapping(Texture::Axis::S, Texture::WrapMode::REPEAT);
        m_label_texture.setWrapping(Texture::Axis::T, Texture::WrapMode::REPEAT);

        m_label_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::NEAREST);
        m_label_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::NEAREST);

        m_label_texture.setTextureData(Texture::Format::RED32UI,
                                       iwidth, iheight,
                                       map_data->getLabelMatrix().lock().get(),
                                       GL_RED_INTEGER);
    }
    m_label_texture.bind(false);
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::beginRender() {
    m_program.use();
    setupUniforms();
    m_texture.activate();
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::render() {
    drawMapVAO();
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::endRender() {
    m_program.use(false);
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::setupUniforms() {
    m_program.uniform("map_texture", m_texture);
}

/**
 * @brief Gets the vertices for the map geometry
 *
 * @details Each vertex is in the form of (X, Y, U, V)
 *
 * @return An array of every vertex
 */
auto MapNormalizer::GUI::GL::MapRenderingViewBase::getMapVertices()
    -> std::array<glm::vec4, 6>
{
    return {
        // Top-Left Triangle
        glm::vec4{0, 1, 0, 1},
        glm::vec4{1, 0, 1, 0},
        glm::vec4{0, 0, 0, 0},

        // Bottom-Right Triangle
        glm::vec4{0, 1, 0, 1},
        glm::vec4{1, 1, 1, 1},
        glm::vec4{1, 0, 1, 0},
    };
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::drawMapVAO() {
    // Draw the map object
    glBindVertexArray(m_vao);
    MN_LOG_GL_ERRORS();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    MN_LOG_GL_ERRORS();

    glBindVertexArray(0);
    MN_LOG_GL_ERRORS();
}

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getMapProgram() -> Program& {
    return m_program;
}

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getMapTexture() -> Texture& {
    return m_texture;
}

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getLabelTexture() -> Texture&
{
    return m_label_texture;
}

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getPrograms() -> ProgramList {
    return { m_program };
}

