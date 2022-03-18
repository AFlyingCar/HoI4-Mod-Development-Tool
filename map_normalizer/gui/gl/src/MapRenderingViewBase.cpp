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
                               getVertexShaderSource()),
                        Shader(Shader::Type::FRAGMENT,
                               getFragmentShaderSource())
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

void MapNormalizer::GUI::GL::MapRenderingViewBase::beginRender() {
    m_program.use();
    setupUniforms();
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::render() {
    drawMapVAO();
}

void MapNormalizer::GUI::GL::MapRenderingViewBase::endRender() {
    m_program.use(false);
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

auto MapNormalizer::GUI::GL::MapRenderingViewBase::getPrograms() -> ProgramList {
    return { m_program };
}

