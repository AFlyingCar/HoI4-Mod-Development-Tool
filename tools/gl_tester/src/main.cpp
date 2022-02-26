
#include <fstream>

#include <SDL.h>

#include <giomm.h>
#include <gtkmm/main.h>

#include "Options.h"
#include "Util.h"

#include "Logger.h"
#include "ConsoleOutputFunctions.h"

#include "Driver.h"

#include "MapDrawingAreaGL.h"

MapNormalizer::ProgramOptions MapNormalizer::prog_opts = {
    0, "", "", false, false, "", "", false, "", false, false, false /* debug */, false
};

/**
 * @brief Mock class which overrides the base Gtk methods
 */
class MockMapDrawingArea: public MapNormalizer::GUI::GL::MapDrawingArea {
    public:
        MockMapDrawingArea(SDL_Window* window, SDL_GLContext context):
            MapDrawingArea(),
            m_window(window),
            m_context(context)
        { }

        ~MockMapDrawingArea() {
            SDL_DestroyWindow(m_window);
            SDL_GL_DeleteContext(m_context);
        }

        virtual void queueDraw() override { }
        virtual void onShow() override { }

        using MapDrawingArea::on_button_press_event;
        using MapDrawingArea::on_render;
    protected:
        virtual void makeCurrent() override {
            SDL_GL_MakeCurrent(m_window, m_context);
        }

        virtual void setSizeRequest(int = -1, int = -1) override {
            // TODO
        }

    private:
        SDL_Window* m_window;
        SDL_GLContext m_context;
};

int main(int argc, char** argv) {
    using namespace MapNormalizer;
    Log::Logger::registerOutputFunction(Log::outputWithFormatting);

    Log::Logger::registerOutputFunction([](const Log::Message& message) {
        static std::ofstream log_file("gl_tester.log");

        return Log::outputToStream(message, false, true,
                                   [&](uint8_t) -> std::ofstream& { return log_file; },
                                   true, true);
    });

    bool done = false;

    // Optional debug
    if(argc > 2) {
        if(std::string(argv[2]) == "--debug") {
            prog_opts.debug = true;
        }
    }

    // Initialize the project file
    GUI::Driver::UniqueProject project(new GUI::Driver::HProject);
    if(argc > 1) {
        std::filesystem::path path = argv[1];
        if(!path.empty()) {
            project->setPath(path);
            if(!project->load()) {
                WRITE_ERROR("Failed to open file.");
                return 1;
            }
        }
    } else {
        WRITE_ERROR("Missing required 'path' argument");
        return 1;
    }
    const auto& map_project = project->getMapProject();

    // Initialize GTK
    Gtk::Main main(argc, argv);

    GUI::Driver::getInstance().initialize();

    auto resource_path = getExecutablePath() / "resources.gresource.c";
    auto resources = Gio::Resource::create_from_file(resource_path.generic_string());

    // Initialize SDL

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        WRITE_ERROR("Failed to initialize SDL: ", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("gl_tester", SDL_WINDOWPOS_CENTERED,
                                                       SDL_WINDOWPOS_CENTERED,
                                                       1280, 720,
                                                       SDL_WINDOW_OPENGL |
                                                       SDL_WINDOW_RESIZABLE | 
                                                       SDL_WINDOW_MAXIMIZED |
                                                       SDL_WINDOW_SHOWN);

    if(window == nullptr) {
        WRITE_ERROR("Failed to create window: ", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Set up OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    auto* context = SDL_GL_CreateContext(window);

    if(context == nullptr) {
        WRITE_ERROR("Failed to create GL context: ", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Fake Gtk context
    Glib::RefPtr<Gdk::GLContext> gtk_context;

    ////////////////////////////////////////////////////////////////////////////
    // GTK RENDERERS BEING TESTED
    ////////////////////////////////////////////////////////////////////////////

    MockMapDrawingArea map_drawing_area(window, context);

    {
        map_drawing_area.setOnProvinceSelectCallback([&map_drawing_area](uint32_t x, uint32_t y)
        {
            if(auto opt_project = GUI::Driver::getInstance().getProject();
                    opt_project)
            {
                auto& project = opt_project->get();
                auto& map_project = project.getMapProject();

                auto map_data = project.getMapProject().getMapData();
                auto lmatrix = project.getMapProject().getLabelMatrix();

                // If the click happens outside of the bounds of the image, then
                //   deselect the province
                if(x > map_data->getWidth() || y > map_data->getHeight()) {
                    map_project.selectProvince(-1);

                    map_drawing_area.setSelection();

                    return;
                }

                // Get the label for the pixel that got clicked on
                auto label = lmatrix[xyToIndex(map_data->getWidth(), x, y)];

                WRITE_DEBUG("Selecting province with ID ", label);
                map_project.selectProvince(label - 1);

                // If the label is a valid province, then go ahead and mark it as
                //  selected everywhere that needs it to be marked as such
                if(auto opt_selected = project.getMapProject().getSelectedProvince();
                        opt_selected)
                {
                    auto* province = &opt_selected->get();
                    auto preview_data = map_project.getPreviewData(province);

                    map_drawing_area.setSelection({preview_data, province->bounding_box, province->id});
                }
            }
        });

        // Call render once to force initialization
        map_drawing_area.on_render(gtk_context);

        // Set up the drawing area
        map_drawing_area.setMapData(map_project.getMapData());

        // We no longer need to own the project, so give it to the Driver
        GUI::Driver::getInstance().setProject(std::move(project));
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    SDL_Event e;
    while(!done) {
        //////// EVENT POLLING ////////
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    {
                        int x, y;
                        SDL_GetMouseState(&x, &y);
                        WRITE_DEBUG("SDL_MOUSEBUTTONDOWN(", x, ", ", y, ")");
                        GdkEventButton button {
                            GDK_BUTTON_PRESS /* type */,
                            nullptr /* window */,
                            1 /* send_event */,
                            0 /* time */,
                            static_cast<gdouble>(x),
                            static_cast<gdouble>(y),
                            nullptr /* axes */,
                            0 /* state */,
                            1 /* button */,
                            nullptr /* device */,
                            0 /* x_root */,
                            0 /* y_root */
                        };
                        map_drawing_area.on_button_press_event(&button);
                    }
                    break;
                case SDL_KEYDOWN:
                    switch(e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
            }
        }

        //////// RENDERING ////////
        map_drawing_area.on_render(gtk_context);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}

