
#include "MapDrawingAreaGL.h"

#include <GL/glew.h>

#include "Logger.h"

#include "Shader.h"

namespace {
// This is an auto-generated header which will look like the following:
#include "GLShaderSources.h"
}

MapNormalizer::GUI::GL::MapDrawingArea::MapDrawingArea()
{ }

MapNormalizer::GUI::GL::MapDrawingArea::~MapDrawingArea() {
}

void MapNormalizer::GUI::GL::MapDrawingArea::on_realize() {
    make_current();

    try {
        throw_if_error();

        init();
    } catch(const Gdk::GLError& error) {
        writeError<true>("An error occurred when making the context current during realize. Reason: ",
                    error.domain(), '-', error.code(), '-', error.what());
    }
}

bool MapNormalizer::GUI::GL::MapDrawingArea::on_render(const Glib::RefPtr<Gdk::GLContext>& context)
{
    try
    {
        throw_if_error();

        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        for(auto&& texture : m_textures[getViewingMode()]) {
            texture->activate();
        }

        m_current_program->use();

        // TODO: Draw map
        glBindVertexArray(m_map_vao);
        // glVertexAttrib1f(0, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        m_current_program->use(false);

        return true;
    }
    catch(const Gdk::GLError& error) {
        writeError<true>("An error occurred when rendering the map",
                    error.domain(), '-', error.code(), '-', error.what());
        return false;
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::on_unrealize() {
    try {
        throw_if_error();

        // Delete buffers and program
    } catch(const Gdk::GLError& error) {
        writeError<true>("An error occurred when making the context current during unrealize. Reason: ",
                    error.domain(), '-', error.code(), '-', error.what());
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::init() {
    // Set up all shaders
    m_provinceview_program = Program{Shader(Shader::Type::VERTEX,
                                            provinceview_vertex),
                                     Shader(Shader::Type::FRAGMENT,
                                            provinceview_fragment)
                                    };

    // NOTE: We could use EBOs here, but we are only going to have this one
    //   object, so there really isn't much of a point

    // gen vertex arrays
    glGenVertexArrays(1, &m_map_vao);

    // bind vertex arrays
    glBindVertexArray(m_map_vao);

    // gen buffers
    glGenBuffers(1, &m_map_vbo);

    // bind buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_map_vbo);

    // buffer data
    auto vertices = getMapVertices();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(*vertices.data()),
                 vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Location 0

    // unbind buffer (bind buffer 0)
    // glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void MapNormalizer::GUI::GL::MapDrawingArea::onZoom() {
    // TODO
}

void MapNormalizer::GUI::GL::MapDrawingArea::onViewingModeChange(ViewingMode viewing_mode)
{
    // Make sure we deactivate any current textures first
    for(auto&& texture : m_textures[getViewingMode()]) {
        texture->deactivate();
    }

    switch(viewing_mode) {
        case ViewingMode::PROVINCE_VIEW:
            m_current_program = &m_provinceview_program;
            break;
        case ViewingMode::STATES_VIEW:
            // TODO
            break;
        default:
            throw std::invalid_argument("Invalid ViewingMode passed to GL::MapDrawingArea::onViewingModeChange");
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::onSetGraphicsData(const unsigned char* data)
{
    if(const auto* image = getImage(); image != nullptr) {
        auto& map_texture = getTexturesFor(ViewingMode::PROVINCE_VIEW).front();

        map_texture->setTextureData(Texture::Format::RGB,
                                    image->info_header.width,
                                    image->info_header.height, data);
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::onSetImage(const BitMap* image) {
    if(const auto* data = getGraphicsData(); data != nullptr) {
        auto& map_texture = getTexturesFor(ViewingMode::PROVINCE_VIEW).front();

        map_texture->setTextureData(Texture::Format::RGB,
                                    image->info_header.width,
                                    image->info_header.height, data);
    }
}

auto MapNormalizer::GUI::GL::MapDrawingArea::getTexturesFor(ViewingMode viewing_mode)
    -> const std::vector<std::shared_ptr<Texture>>&
{
    auto& textures = m_textures[viewing_mode];

    if(textures.empty()) {
        switch(viewing_mode) {
            case ViewingMode::PROVINCE_VIEW:
                {
                    std::shared_ptr<Texture> texture(new Texture);

                    texture->setTarget(Texture::Target::TEX_2D);
                    // We don't set any data on this texture because it is dependent
                    //  on the graphics/image data

                    textures.push_back(texture);
                }
                break;
            case ViewingMode::STATES_VIEW:
                // TODO
                break;
        }
    }

    return m_textures[viewing_mode];
}

auto MapNormalizer::GUI::GL::MapDrawingArea::getMapVertices()
    -> std::array<glm::vec4, 6>
{
    auto&& image = getImage();

    auto width = static_cast<uint32_t>(image->info_header.width);
    auto height = static_cast<uint32_t>(image->info_header.height);

    // The image is going to be a single square
    return {
        // Top-Left Triangle
        glm::vec4{0, 0,          0, 0},
        glm::vec4{0, height,     0, 1},
        glm::vec4{width, 0,      1, 0},

        // Bottom-Right Triangle
        glm::vec4{width, 0,      1, 0},
        glm::vec4{0, height,     0, 1},
        glm::vec4{width, height, 1, 1}
    };
}

