/**
 * @file MapRenderingViewBase.h
 *
 * @brief A base class for rendering just the map by itself.
 */

#include "MapRenderingViewBase.h"

#include <GL/glew.h>

#include "Logger.h"

#include "Driver.h"
#include "Util.h"

#include "GLUtils.h"
#include "GLShaderSources.h"
#include "GuiUtils.h"

HMDT::GUI::GL::MapRenderingViewBase::MapRenderingViewBase() {
}

HMDT::GUI::GL::MapRenderingViewBase::~MapRenderingViewBase() {
    if(m_vao != -1) {
        glDeleteVertexArrays(1, &m_vao);
        HMDT_LOG_GL_ERRORS();
    }
}

/**
 * @brief Initializes this MapRenderingViewBase
 *
 * @details Will build the basic shader program as well as build the geometry
 *          that will be used for drawing the map
 */
void HMDT::GUI::GL::MapRenderingViewBase::init() {
    // Set up all shaders
    WRITE_DEBUG("Building map rendering view program...");
    m_program = Program{Shader(Shader::Type::VERTEX,
                               getVertexShaderSource()),
                        Shader(Shader::Type::FRAGMENT,
                               getFragmentShaderSource())
                       };
    HMDT_LOG_GL_ERRORS();

    WRITE_DEBUG("Building map object...");
    {
        // NOTE: We could use EBOs here, but we are only going to have this one
        //   object, so there really isn't much of a point

        // gen vertex arrays
        glGenVertexArrays(1, &m_vao);
        HMDT_LOG_GL_ERRORS();

        // gen buffers
        glGenBuffers(1, &m_vbo);
        HMDT_LOG_GL_ERRORS();

        // bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        HMDT_LOG_GL_ERRORS();

        // buffer data
        auto vertices = getMapVertices();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(*vertices.data()),
                     vertices.data(), GL_STATIC_DRAW);
        HMDT_LOG_GL_ERRORS();

        // bind vertex arrays
        glBindVertexArray(m_vao);
        HMDT_LOG_GL_ERRORS();

        glEnableVertexAttribArray(0); // Location 0
        HMDT_LOG_GL_ERRORS();
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        HMDT_LOG_GL_ERRORS();

        // unbind buffer and vertex array
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        HMDT_LOG_GL_ERRORS();
        glBindVertexArray(0);
        HMDT_LOG_GL_ERRORS();
    }

    WRITE_DEBUG("Building Selection Texture...");
    {
        // TODO: This path shouldn't be hardcoded, we should get it instead from
        //   the build system/from a ResourceManager
        auto stream = Driver::getInstance().getResources()->open_stream("/com/aflyingcar/HoI4ModDevelopmentTool/textures/selection.bmp");

        std::unique_ptr<BitMap> selection_bmp(new BitMap);
        if(readBMP(stream, selection_bmp.get()) == nullptr) {
            WRITE_ERROR("Failed to load selection texture!");

            m_selection_texture.setTextureUnitID(Texture::Unit::TEX_UNIT3);

            m_selection_texture.bind();

            m_selection_texture.setTextureData(Texture::Format::RGBA, 1, 1,
                                               (uint8_t*)0);
            m_selection_texture.bind(false);
        } else {
            auto iwidth = selection_bmp->info_header.width;
            auto iheight = selection_bmp->info_header.height;

            m_selection_texture.setTextureUnitID(Texture::Unit::TEX_UNIT3);

            m_selection_texture.bind();
            {
                // Use NEAREST rather than LINEAR to prevent weird outlines around
                //  the textures
                m_selection_texture.setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
                m_selection_texture.setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

                m_selection_texture.setWrapping(Texture::Axis::S, Texture::WrapMode::REPEAT);
                m_selection_texture.setWrapping(Texture::Axis::T, Texture::WrapMode::REPEAT);

                m_selection_texture.setTextureData(Texture::Format::RGBA,
                                                   iwidth, iheight,
                                                   selection_bmp->data);
            }
            m_selection_texture.bind(false);
        }
    }
}

void HMDT::GUI::GL::MapRenderingViewBase::beginRender() {
    m_program.use();
    setupUniforms();
}

void HMDT::GUI::GL::MapRenderingViewBase::render() {
    drawMapVAO();
}

void HMDT::GUI::GL::MapRenderingViewBase::endRender() {
    m_program.use(false);
}

/**
 * @brief Gets the vertices for the map geometry
 *
 * @details Each vertex is in the form of (X, Y, U, V)
 *
 * @return An array of every vertex
 */
auto HMDT::GUI::GL::MapRenderingViewBase::getMapVertices()
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

auto HMDT::GUI::GL::MapRenderingViewBase::getSelectionTexture() -> Texture& {
    return m_selection_texture;
}

void HMDT::GUI::GL::MapRenderingViewBase::drawMapVAO() {
    // Draw the map object
    glBindVertexArray(m_vao);
    HMDT_LOG_GL_ERRORS();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    HMDT_LOG_GL_ERRORS();

    glBindVertexArray(0);
    HMDT_LOG_GL_ERRORS();
}

auto HMDT::GUI::GL::MapRenderingViewBase::getMapProgram() -> Program& {
    return m_program;
}

auto HMDT::GUI::GL::MapRenderingViewBase::getPrograms() -> ProgramList {
    return { m_program };
}

