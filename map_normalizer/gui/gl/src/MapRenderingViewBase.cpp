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
        WRITE_DEBUG("glGenVertexArrays(1, ", &m_vao, ')');
        glGenVertexArrays(1, &m_vao);
        MN_LOG_GL_ERRORS();

        // gen buffers
        WRITE_DEBUG("glGenBuffers(1, ", &m_vbo, ')');
        glGenBuffers(1, &m_vbo);
        MN_LOG_GL_ERRORS();

        // bind buffer
        WRITE_DEBUG("glBindBuffer(GL_ARRAY_BUFFER, ", m_vbo, ')');
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        MN_LOG_GL_ERRORS();

        // buffer data
        auto vertices = getMapVertices();
        WRITE_DEBUG("glBufferData(GL_ARRAY_BUFFER, ",
                          vertices.size() * sizeof(*vertices.data()), ',',
                          vertices.data(), ",GL_STATIC_DRAW)");
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(*vertices.data()),
                     vertices.data(), GL_STATIC_DRAW);
        MN_LOG_GL_ERRORS();

        // bind vertex arrays
        WRITE_DEBUG("glBindVertexArray(", m_vao, ')');
        glBindVertexArray(m_vao);
        MN_LOG_GL_ERRORS();

        WRITE_DEBUG("glEnableVertexAttribArray(0)");
        glEnableVertexAttribArray(0); // Location 0
        MN_LOG_GL_ERRORS();
        WRITE_DEBUG("glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, ", 4 * sizeof(float), ", (void*)0)");
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        MN_LOG_GL_ERRORS();

        // unbind buffer and vertex array
        WRITE_DEBUG("glBindBuffer(GL_ARRAY_BUFFER, 0)");
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        MN_LOG_GL_ERRORS();
        WRITE_DEBUG("glBindVertexArray(0)");
        glBindVertexArray(0);
        MN_LOG_GL_ERRORS();
    }
}

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
    m_program.uniform("map_texture", 0);
}

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
    WRITE_DEBUG("glBindVertexArray(", m_vao, ")");
    glBindVertexArray(m_vao);
    MN_LOG_GL_ERRORS();

    WRITE_DEBUG("glBindVertexArray(GL_TRIANGLES, 0, 6)");
    glDrawArrays(GL_TRIANGLES, 0, 6);
    MN_LOG_GL_ERRORS();

    WRITE_DEBUG("glBindVertexArray(0)");
    glBindVertexArray(0);
    MN_LOG_GL_ERRORS();
}

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getMapProgram() -> Program& {
    return m_program;
}

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getMapTexture() -> Texture& {
    return m_texture;
}

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getPrograms() -> ProgramList {
    return { m_program };
}

