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
    writeDebug<true>("Building map rendering view program...");
    m_program = Program{Shader(Shader::Type::VERTEX,
                               ShaderSources::provinceview_vertex),
                        Shader(Shader::Type::FRAGMENT,
                               ShaderSources::provinceview_fragment)
                       };
    MN_LOG_GL_ERRORS();

    writeDebug<true>("Building map object...");
    {
        // NOTE: We could use EBOs here, but we are only going to have this one
        //   object, so there really isn't much of a point

        // gen vertex arrays
        writeDebug<true>("glGenVertexArrays(1, ", &m_vao, ')');
        glGenVertexArrays(1, &m_vao);
        MN_LOG_GL_ERRORS();

        // gen buffers
        writeDebug<true>("glGenBuffers(1, ", &m_vbo, ')');
        glGenBuffers(1, &m_vbo);
        MN_LOG_GL_ERRORS();

        // bind buffer
        writeDebug<true>("glBindBuffer(GL_ARRAY_BUFFER, ", m_vbo, ')');
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        MN_LOG_GL_ERRORS();

        // buffer data
        auto vertices = getMapVertices();
        writeDebug<true>("glBufferData(GL_ARRAY_BUFFER, ",
                          vertices.size() * sizeof(*vertices.data()), ',',
                          vertices.data(), ",GL_STATIC_DRAW)");
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(*vertices.data()),
                     vertices.data(), GL_STATIC_DRAW);
        MN_LOG_GL_ERRORS();

        // bind vertex arrays
        writeDebug<true>("glBindVertexArray(", m_vao, ')');
        glBindVertexArray(m_vao);
        MN_LOG_GL_ERRORS();

        writeDebug<true>("glEnableVertexAttribArray(0)");
        glEnableVertexAttribArray(0); // Location 0
        MN_LOG_GL_ERRORS();
        writeDebug<true>("glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, ", 4 * sizeof(float), ", (void*)0)");
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        MN_LOG_GL_ERRORS();

        // unbind buffer and vertex array
        writeDebug<true>("glBindBuffer(GL_ARRAY_BUFFER, 0)");
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        MN_LOG_GL_ERRORS();
        writeDebug<true>("glBindVertexArray(0)");
        glBindVertexArray(0);
        MN_LOG_GL_ERRORS();
    }
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::onMapDataChanged(const BitMap* image,
                                                                    const unsigned char* data)
{
    auto iwidth = image->info_header.width;
    auto iheight = image->info_header.height;

    m_texture.setTextureUnitID(Texture::Unit::TEX_UNIT0);

    m_texture.bind();
    {
        // map_texture->setWrapping(Texture::Axis::S, Texture::WrapMode::CLAMP_TO_EDGE);
        // map_texture->setWrapping(Texture::Axis::T, Texture::WrapMode::CLAMP_TO_EDGE);

        m_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
        m_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

        m_texture.setTextureData(Texture::Format::RGB,
                                  iwidth, iheight, data);
    }
    m_texture.bind(false);
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::beginRender() {
    m_program.use();
    setupUniforms();
    m_texture.activate();
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::render() {
    // Draw the map object
    writeDebug<true>("glBindVertexArray(", m_vao, ")");
    glBindVertexArray(m_vao);
    MN_LOG_GL_ERRORS();

    writeDebug<true>("glBindVertexArray(GL_TRIANGLES, 0, 6)");
    glDrawArrays(GL_TRIANGLES, 0, 6);
    MN_LOG_GL_ERRORS();

    writeDebug<true>("glBindVertexArray(0)");
    glBindVertexArray(0);
    MN_LOG_GL_ERRORS();
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

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getProgram() -> Program& {
    return m_program;
}

