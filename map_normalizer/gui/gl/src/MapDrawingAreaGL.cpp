
#include "MapDrawingAreaGL.h"

#include <gtkmm.h>

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Logger.h"
#include "Constants.h"

#include "GLEWInitializationException.h"
#include "Shader.h"
#include "GLUtils.h"

namespace {
// This is an auto-generated header which will look like the following:
#include "GLShaderSources.h"
}

MapNormalizer::GUI::GL::MapDrawingArea::MapDrawingArea():
    m_initialized(false)
{ }

MapNormalizer::GUI::GL::MapDrawingArea::~MapDrawingArea() {
    glDeleteVertexArrays(1, &m_map_vao);
    MN_LOG_GL_ERRORS();
}

bool MapNormalizer::GUI::GL::MapDrawingArea::on_render(const Glib::RefPtr<Gdk::GLContext>& context)
{
    try
    {
        make_current();
        init();

        throw_if_error();

        writeDebug<true>("Re-drawing...");

        // Clear the current screen
        glClearColor(0.5, 0.5, 0.5, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        MN_LOG_GL_ERRORS();

        // Do nothing if we have no graphics data to actually render
        if(!hasData()) {
            return true;
        }

        // As a note, Gtk sets the viewport for us, so we don't need to worry
        //  about doing it ourselves

        // Draw the scene
        {
            m_current_program->use();

            setupAllUniforms();

            for(auto&& texture : m_textures[getViewingMode()]) {
                texture->activate();
            }

            // Draw the map object once all uniforms and textures are set up
            writeDebug<true>("glBindVertexArray(", m_map_vao, ")");
            glBindVertexArray(m_map_vao);
            MN_LOG_GL_ERRORS();

            writeDebug<true>("glBindVertexArray(GL_TRIANGLES, 0, 6)");
            glDrawArrays(GL_TRIANGLES, 0, 6);
            MN_LOG_GL_ERRORS();

            writeDebug<true>("glBindVertexArray(0)");
            glBindVertexArray(0);
            MN_LOG_GL_ERRORS();

            m_current_program->use(false);
        }

        glFlush();
        MN_LOG_GL_ERRORS();

        return true;
    }
    catch(const Gdk::GLError& error) {
        writeError<true>("An error occurred when rendering the map",
                    error.domain(), '-', error.code(), '-', error.what());
        return false;
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::on_unrealize() {
    make_current();

    try {
        throw_if_error();

        // Delete buffers and program
    } catch(const Gdk::GLError& error) {
        writeError<true>("An error occurred when making the context current during unrealize. Reason: ",
                    error.domain(), '-', error.code(), '-', error.what());
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::init() {
    if(m_initialized) return;

    glewExperimental = true;

    if(auto result = glewInit(); result != GLEW_OK) {
        throw GLEWInitializationException(result);
    }

    // Set up all shaders
    writeDebug<true>("Building all programs...");
    m_provinceview_program = Program{Shader(Shader::Type::VERTEX,
                                            provinceview_vertex),
                                     Shader(Shader::Type::FRAGMENT,
                                            provinceview_fragment)
                                    };
    MN_LOG_GL_ERRORS();

    writeDebug<true>("Building map object...");
    {
        // NOTE: We could use EBOs here, but we are only going to have this one
        //   object, so there really isn't much of a point

        // gen vertex arrays
        writeDebug<true>("glGenVertexArrays(1, ", &m_map_vao, ')');
        glGenVertexArrays(1, &m_map_vao);
        MN_LOG_GL_ERRORS();

        // gen buffers
        writeDebug<true>("glGenBuffers(1, ", &m_map_vbo, ')');
        glGenBuffers(1, &m_map_vbo);
        MN_LOG_GL_ERRORS();

        // bind buffer
        writeDebug<true>("glBindBuffer(GL_ARRAY_BUFFER, ", m_map_vbo, ')');
        glBindBuffer(GL_ARRAY_BUFFER, m_map_vbo);
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
        writeDebug<true>("glBindVertexArray(", m_map_vao, ')');
        glBindVertexArray(m_map_vao);
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

    onViewingModeChange(DEFAULT_VIEWING_MODE);

    m_initialized = true;
}

void MapNormalizer::GUI::GL::MapDrawingArea::onZoom() {
    // Make sure we update the size request of the image
    if(hasData()) {
        auto siwidth = getImage()->info_header.width * getScaleFactor();
        auto siheight = getImage()->info_header.height * getScaleFactor();
        set_size_request(siwidth, siheight);

        queue_draw();
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::onViewingModeChange(ViewingMode viewing_mode)
{
    switch(viewing_mode) {
        case ViewingMode::PROVINCE_VIEW:
            m_current_program = &m_provinceview_program;
            break;
        case ViewingMode::STATES_VIEW:
            // TODO
            break;
        default:
            using namespace std::string_literals;
            throw std::invalid_argument(("Invalid ViewingMode passed to GL::MapDrawingArea::onViewingModeChange: "s + std::to_string(static_cast<int>(viewing_mode))).c_str());
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::onSetData(const BitMap* image,
                                                       const unsigned char* data)
{
    if(image != nullptr && data != nullptr) {
        writeDebug<true>("Setting map texture data");

        auto& map_texture = getTexturesFor(ViewingMode::PROVINCE_VIEW).front();

        map_texture->setTextureUnitID(Texture::Unit::TEX_UNIT0);

        auto iwidth = image->info_header.width;
        auto iheight = image->info_header.height;

        map_texture->bind();
        {
            // map_texture->setWrapping(Texture::Axis::S, Texture::WrapMode::CLAMP_TO_EDGE);
            // map_texture->setWrapping(Texture::Axis::T, Texture::WrapMode::CLAMP_TO_EDGE);

            map_texture->setFiltering(Texture::FilterType::MAG, Texture::Filter::LINEAR);
            map_texture->setFiltering(Texture::FilterType::MIN, Texture::Filter::LINEAR);

            map_texture->setTextureData(Texture::Format::RGB,
                                        iwidth, iheight, data);
        }
        map_texture->bind(false);

        auto siwidth = iwidth * getScaleFactor();
        auto siheight = iheight * getScaleFactor();
        set_size_request(siwidth, siheight);
    } else {
        // We are no longer worrying about this texture
        m_textures[ViewingMode::PROVINCE_VIEW].clear();
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::onShow() {
    set_required_version(4, 10); // Must be called before the area has been realized
}

void MapNormalizer::GUI::GL::MapDrawingArea::queueDraw() {
    queue_draw();
    show_all();
}

auto MapNormalizer::GUI::GL::MapDrawingArea::getTexturesFor(ViewingMode viewing_mode)
    -> const std::vector<std::shared_ptr<Texture>>&
{
    auto& textures = m_textures[viewing_mode];

    // Initialize the textures if they haven't been initialized yet
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

void MapNormalizer::GUI::GL::MapDrawingArea::setupAllUniforms() {
    switch(getViewingMode()) {
        case ViewingMode::PROVINCE_VIEW:
            setupProvinceViewUniforms();
            break;
        case ViewingMode::STATES_VIEW:
            setupStatesViewUniforms();
            break;
        default:
            using namespace std::string_literals;
            throw std::invalid_argument(("Invalid viewing_mode"s + std::to_string(static_cast<int>(getViewingMode()))).c_str());
    }
}

void MapNormalizer::GUI::GL::MapDrawingArea::setupProvinceViewUniforms() {
    if(!hasData()) {
        // TODO: writeError, this shouldn't happen
        return;
    }

    writeDebug<true>("setupProvinceViewUniforms()");

    m_current_program->use();

    // Set up the projection matrix
    m_current_program->uniform("projection", getProjection());

    // Set up the transformation matrix
    m_current_program->uniform("transform", getTransformation());

    // Set up the map texture sampler
    m_current_program->uniform("map_texture", 0);
}

glm::mat4 MapNormalizer::GUI::GL::MapDrawingArea::getProjection() {
    auto&& image = getImage();

    // The dimensions of the image
    glm::vec2 size{image->info_header.width, image->info_header.height};

    // Make sure that we scale the projection as well
    size *= getScaleFactor();

    // Note that we flip bottom and top to prevent the image from also being
    //   flipped.
    // left, right, bottom, top
    return glm::ortho(0.0f, static_cast<float>(size.x), static_cast<float>(size.y), 0.0f);
}

glm::mat4 MapNormalizer::GUI::GL::MapDrawingArea::getTransformation() {
    auto&& image = getImage();

    // The dimensions of the image
    glm::vec2 size{image->info_header.width, image->info_header.height};

    glm::mat4 transform = glm::mat4{1.0f};

    // TODO: Store translation as a position value (we want to be able to move this thing later
    transform = glm::translate(transform, glm::vec3{0, 0, 0});

    transform = glm::scale(transform, glm::vec3{size, 1} *
                                      glm::vec3{getScaleFactor(),
                                                getScaleFactor(), 1}
    );

    return transform;
}

bool MapNormalizer::GUI::GL::MapDrawingArea::on_button_press_event(GdkEventButton* event)
{
    if(!hasData()) {
        return true;
    }

    // Is it a left-click?
    if(event->type == GDK_BUTTON_PRESS && event->button == 1) {
        // Note that x and y will be the values after scaling. If we want the
        //  true coordinates, we have to invert the scaling
#if 1
        auto x = event->x * (1 / getScaleFactor());
        auto y = event->y * (1 / getScaleFactor());
#else
        // TODO: We should somehow do the reverse transformation as matrix math
        //   so we can do other stuff later (like translation) without needing
        //   to reinvent the wheel
        auto xy_ = glm::inverse(getTransformation()) * glm::vec4{event->x,
                                                                 event->y,
                                                                 1, 1};
        auto x = xy_.x;
        auto y = xy_.y;
#endif

        if(event->state & GDK_SHIFT_MASK) {
            getOnMultiSelect()(x, y);
        } else {
            getOnSelect()(x, y);
        }
    }

    return true;
}

void MapNormalizer::GUI::GL::MapDrawingArea::setupStatesViewUniforms() {
}

