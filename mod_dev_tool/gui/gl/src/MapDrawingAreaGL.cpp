/**
 * @file MapDrawingAreaGL.cpp
 *
 * @brief Defines the OpenGL map drawing area
 */

#include "MapDrawingAreaGL.h"

#include <gtkmm.h>

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Logger.h"
#include "Constants.h"
#include "Options.h"

#include "GLEWInitializationException.h"
#include "Shader.h"
#include "GLUtils.h"
#include "ProvinceRenderingView.h"
#include "StateRenderingView.h"

HMDT::GUI::GL::MapDrawingArea::MapDrawingArea():
    m_initialized(false)
{ }

HMDT::GUI::GL::MapDrawingArea::~MapDrawingArea() { }

/**
 * @brief Renders the current IRenderingView
 *
 * @param context The OpenGL context
 *
 * @return true if no Gdk::GLErrors were thrown.
 */
bool HMDT::GUI::GL::MapDrawingArea::on_render(const Glib::RefPtr<Gdk::GLContext>& context)
{
    try {
        makeCurrent();
        init();

        throw_if_error();

        // Clear the current screen
        glClearColor(0.5, 0.5, 0.5, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        HMDT_LOG_GL_ERRORS();

        // Do nothing if we have no graphics data to actually render
        if(!hasData()) {
            return true;
        }

        // As a note, Gtk sets the viewport for us, so we don't need to worry
        //  about doing it ourselves

        // Draw the scene
        auto current_rendering_view = getCurrentRenderingView();

        setupAllUniforms();

        current_rendering_view->beginRender();
        current_rendering_view->render();
        current_rendering_view->endRender();

        glFlush();
        HMDT_LOG_GL_ERRORS();

        // Make sure that we re-draw so that we don't get any weird flickering/
        //  artifacts
        queue_draw();

        return true;
    } catch(const Gdk::GLError& error) {
        WRITE_ERROR("An error occurred when rendering the map",
                    error.domain(), '-', error.code(), '-', error.what());
        return false;
    }
}

void HMDT::GUI::GL::MapDrawingArea::on_unrealize() {
    makeCurrent();

    try {
        throw_if_error();

        // Delete buffers and program
    } catch(const Gdk::GLError& error) {
        WRITE_ERROR("An error occurred when making the context current during unrealize. Reason: ",
                    error.domain(), '-', error.code(), '-', error.what());
    }
}

/**
 * @brief Initializes the drawing area
 */
void HMDT::GUI::GL::MapDrawingArea::init() {
    if(m_initialized) return;

    glewExperimental = true;

    if(auto result = glewInit(); result != GLEW_OK) {
        throw GLEWInitializationException(result);
    }

    // Enable all of our settings that we care about
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initShaderMacros();

    if(prog_opts.debug) {
        WRITE_DEBUG("Initializing GL Debug Output functions");
        glEnable(GL_DEBUG_OUTPUT);

        glDebugMessageCallback(+[](GLenum source, GLenum type, GLuint id,
                                   GLenum severity, GLsizei length,
                                   const GLchar* message, const void* /*userParam*/)
        {
            using namespace Log;

            if(type == GL_DEBUG_TYPE_ERROR) {
                // SEV=0x{SEVERITY}, {MESSAGE}
                Logger::getInstance().writeError(
                    Source("OPENGL"),
                    "SEV=0x", std::hex, static_cast<uint32_t>(severity), std::dec,
                    ", ", message);
            } else {
                Logger::getInstance().writeDebug(
                    Source("OPENGL"),
                    "SEV=0x", std::hex, static_cast<uint32_t>(severity), std::dec,
                    ", ", message);
            }
        }, nullptr);
        WRITE_DEBUG("Done.");
    }

    m_rendering_views[ViewingMode::PROVINCE_VIEW].reset(new ProvinceRenderingView());
    m_rendering_views[ViewingMode::STATES_VIEW].reset(new StateRenderingView());

    WRITE_DEBUG("Initializing each rendering view.");
    for(auto&& [viewing_mode, rendering_view] : m_rendering_views) {
        WRITE_DEBUG("Initializing ", std::to_string(static_cast<int>(viewing_mode)));
        rendering_view->init();
        rendering_view->m_owning_gl_drawing_area = this;
    }

    onViewingModeChange(DEFAULT_VIEWING_MODE);

    m_initialized = true;
}

/**
 * @brief Will adjust the size of the drawable window and re-draw
 */
void HMDT::GUI::GL::MapDrawingArea::onZoom() {
    // Make sure we update the size request of the image
    if(hasData()) {
        auto [iwidth, iheight] = getMapData()->getDimensions();
        auto siwidth = iwidth * getScaleFactor();
        auto siheight = iheight * getScaleFactor();
        setSizeRequest(siwidth, siheight);

        queue_draw();
    }
}

/**
 * @brief Adjusts the size of the drawable window for the new data.
 *
 * @details Will also invoke IRenderingView::onMapDataChanged() for every
 *          rendering view
 *
 * @param map_data The new map data. May be null.
 */
void HMDT::GUI::GL::MapDrawingArea::onSetData(std::shared_ptr<const MapData> map_data)
{
    if(map_data == nullptr) return;

    WRITE_DEBUG("Setting map texture data");

    auto [iwidth, iheight] = map_data->getDimensions();

    WRITE_DEBUG("Updating map data for each rendering view.");
    for(auto&& [viewing_mode, rendering_view] : m_rendering_views) {
        WRITE_DEBUG("Initializing ", std::to_string(static_cast<int>(viewing_mode)));
        rendering_view->onMapDataChanged(map_data);
    }

    auto siwidth = iwidth * getScaleFactor();
    auto siheight = iheight * getScaleFactor();
    setSizeRequest(siwidth, siheight);
}

void HMDT::GUI::GL::MapDrawingArea::onShow() {
    // Must be called before the area has been realized
    set_required_version(4, 10);
}

void HMDT::GUI::GL::MapDrawingArea::onSelectionChanged(std::optional<SelectionInfo> selection)
{
    getCurrentRenderingView()->onSelectionChanged(selection);

    queue_draw();
}

void HMDT::GUI::GL::MapDrawingArea::queueDraw() {
    queue_draw();
    show_all();
}

/**
 * @brief Initializes all global shader macros
 */
void HMDT::GUI::GL::MapDrawingArea::initShaderMacros() {
    Shader::defineMacro("MAX_SELECTED_PROVINCES", MAX_SELECTED_PROVINCES);
}

auto HMDT::GUI::GL::MapDrawingArea::getCurrentRenderingView()
    -> std::shared_ptr<IRenderingView>
{
    return m_rendering_views.at(getViewingMode());
}

/**
 * @brief Prepares some basic uniforms for each program for the current
 *        rendering view
 */
void HMDT::GUI::GL::MapDrawingArea::setupAllUniforms() {
    auto current_rendering_view = getCurrentRenderingView();

    for(auto& program : current_rendering_view->getPrograms()) {
        // Set up the projection matrix
        program.get().uniform("projection", getProjection());

        // Set up the transformation matrix
        program.get().uniform("transform", getTransformation());
    }
}

/**
 * @brief Gets an orthographic projection based on the current drawable
 *        dimensions.
 */
glm::mat4 HMDT::GUI::GL::MapDrawingArea::getProjection() const {
    auto [iwidth, iheight] = getMapData()->getDimensions();

    // The dimensions of the image
    glm::vec2 size{iwidth, iheight};

    // Make sure that we scale the projection as well
    size *= getScaleFactor();

    // Note that we flip bottom and top to prevent the image from also being
    //   flipped.
    // left, right, bottom, top
    return glm::ortho(0.0f, static_cast<float>(size.x),
                            static_cast<float>(size.y), 0.0f);
}

glm::mat4 HMDT::GUI::GL::MapDrawingArea::getTransformation() const {
    auto [iwidth, iheight] = getMapData()->getDimensions();

    // The dimensions of the image
    glm::vec2 size{iwidth, iheight};

    glm::mat4 transform = glm::mat4{1.0f};

    // TODO: Store translation as a position value (we want to be able to move this thing later
    transform = glm::translate(transform, glm::vec3{0, 0, 0});

    transform = glm::scale(transform, glm::vec3{size, 1} *
                                      glm::vec3{getScaleFactor(),
                                                getScaleFactor(), 1});

    return transform;
}

/**
 * @brief Callback for a mouse-click. Will call the relevant onSelection
 *        callback, depending on if the click was done while SHIFT was held.
 *
 * @param event The mouse-button event
 *
 * @return true
 */
bool HMDT::GUI::GL::MapDrawingArea::on_button_press_event(GdkEventButton* event)
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

void HMDT::GUI::GL::MapDrawingArea::makeCurrent() {
    make_current();
}

