#include "MainWindow.h"

#include <thread>
#include <sstream>

#include "gtkmm.h"
#include "gtkmm/filechooserdialog.h"

#include "BitMap.h"
#include "Constants.h"
#include "Logger.h"
#include "Util.h" // overloaded

#include "ShapeFinder2.h" // ShapeFinder

#include "GraphicalDebugger.h"
#include "MapNormalizerApplication.h"
#include "ProgressBarDialog.h"
#include "NewProjectDialog.h"
#include "Driver.h"

/**
 * @brief Constructs the main window.
 *
 * @param application The application that this window is a part of
 */
MapNormalizer::GUI::MainWindow::MainWindow(Gtk::Application& application):
    Window(APPLICATION_NAME, application)
{
    set_size_request(512, 512);
}

MapNormalizer::GUI::MainWindow::~MainWindow() { }

/**
 * @brief Initializes every action for the menubar
 *
 * @return true
 */
bool MapNormalizer::GUI::MainWindow::initializeActions() {
    initializeFileActions();
    initializeEditActions();
    initializeViewActions();
    initializeProjectActions();

    return true;
}

/**
 * @brief Initializes every action in the File menu
 */
void MapNormalizer::GUI::MainWindow::initializeFileActions() {
    add_action("new", [this]() {
        newProject();
    });

    add_action("open", [this]() {
        openProject();
    });

    auto save_action = add_action("save", [this]() {
        if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
            auto& project = opt_project->get();

            if(!std::filesystem::exists(project.getPath())) {
                writeDebug("SaveProjectAs(", project.getPath(), ")");
                saveProjectAs();
            } else {
                writeDebug("SaveProject(", project.getPath(), ")");
                saveProject();
            }
        }
    });
    save_action->set_enabled(false);

    auto close_action = add_action("close", [this]() {
        Driver::getInstance().setProject();

        onProjectClosed();
    });
    close_action->set_enabled(false);

    add_action("quit", []() {
        // TODO: Confirmation menu first
        std::exit(0);
    });
}

/**
 * @brief Initializes every action in the Edit menu
 */
void MapNormalizer::GUI::MainWindow::initializeEditActions() {
}

/**
 * @brief Initializes every action in the View menu
 */
void MapNormalizer::GUI::MainWindow::initializeViewActions() {
    add_action_bool("properties", [this]() {
        auto self = lookup_action("properties");
        bool active = false;
        self->get_state(active);
        self->change_state(!active);

        if(m_paned->get_child2() == nullptr) {
            buildPropertiesPane();
            m_paned->show_all();
        } else {
            m_paned->remove(*m_paned->get_child2());
        }
    }, false);
}

/**
 * @brief Initializes every action in the Project menu
 */
void MapNormalizer::GUI::MainWindow::initializeProjectActions() {
    auto ipm_action = add_action("import_provincemap", [this]() {
        // Allocate this on the stack so that it gets automatically cleaned up
        //  when we finish
        Gtk::FileChooserDialog dialog(*this, "Choose an input image file");
        dialog.set_select_multiple(false);
        // dialog.add_filter(); // TODO: Filter only for supported file types

        dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Select", Gtk::RESPONSE_ACCEPT);

        const int result = dialog.run();

        switch(result) {
            case Gtk::RESPONSE_ACCEPT:
                dialog.hide(); // Hide ourselves immediately
                if(!importProvinceMap(dialog.get_filename())) {
                    Gtk::MessageDialog dialog("Failed to open file.", false,
                                              Gtk::MESSAGE_ERROR);
                    dialog.run();
                }
                break;
            case Gtk::RESPONSE_CANCEL:
            case Gtk::RESPONSE_DELETE_EVENT:
            default:
                break;
        }
    });

    // This action should be disabled by default, until a project gets opened
    ipm_action->set_enabled(false);
}

/**
 * @brief Initializes every widget used by this window
 *
 * @return true
 */
bool MapNormalizer::GUI::MainWindow::initializeWidgets() {
    m_paned = addWidget<Gtk::Paned>();
    
    // Set the minimum size of the pane to 512x512
    m_paned->set_size_request(MINIMUM_WINDOW_W, MINIMUM_WINDOW_H);

    // Make sure that we take up all of the available vertical space
    m_paned->set_vexpand();

    // The view pane will always be loaded, so load it now
    buildViewPane();

    m_active_child = std::monostate{};

    return true;
}

/**
 * @brief Builds the view pane, which is where the map gets rendered to.
 */
void MapNormalizer::GUI::MainWindow::buildViewPane() {
    m_paned->pack1(*std::get<Gtk::Frame*>(m_active_child = new Gtk::Frame()), true, false);

    // Setup the box+area for the map image to render
    auto drawing_window = addWidget<Gtk::ScrolledWindow>();
    auto drawing_area = m_drawing_area = new MapDrawingArea();

    // Place the drawing area in a scrollable window
    drawing_window->add(*drawing_area);
    drawing_window->show_all();
}

/**
 * @brief Builds the properties pane, which is where properties about a selected
 *        province/state will go.
 *
 * @return The frame that the properties will be placed into
 */
Gtk::Frame* MapNormalizer::GUI::MainWindow::buildPropertiesPane() {
    Gtk::Frame* properties_frame = new Gtk::Frame();
    m_active_child = properties_frame;

    m_paned->pack2(*properties_frame, false, false);

    auto properties_tab = addWidget<Gtk::Notebook>();

    {
        // We want to possibly be able to scroll in the properties window
        auto properties_window = new Gtk::ScrolledWindow();
        properties_window->set_size_request(MINIMUM_PROPERTIES_PANE_WIDTH, -1);

        properties_tab->append_page(*properties_window, "Province");

        // Do this so all future widgets we add get put into this one
        m_active_child = properties_window;

        // TODO: Add all of the properties stuff
        //   We may want a custom widget that knows what is currently selected?
        addWidget<Gtk::Label>("Province Properties");
    }

    {
        // We want to possibly be able to scroll in the properties window
        auto properties_window = new Gtk::ScrolledWindow();
        properties_window->set_size_request(MINIMUM_PROPERTIES_PANE_WIDTH, -1);

        properties_tab->append_page(*properties_window, "State");

        // Do this so all future widgets we add get put into this one
        m_active_child = properties_window;

        // TODO: Add all of the properties stuff
        //   We may want a custom widget that knows what is currently selected?
        addWidget<Gtk::Label>("State Properties");
    }

    return properties_frame;
}

Gtk::Orientation MapNormalizer::GUI::MainWindow::getDisplayOrientation() const {
    return Gtk::Orientation::ORIENTATION_VERTICAL;
}

/**
 * @brief Adds the given widget to the currently active widget
 *
 * @param widget The widget to add
 */
void MapNormalizer::GUI::MainWindow::addWidgetToParent(Gtk::Widget& widget) {
    std::visit(overloaded {
        [&widget](auto&& child) {
            child->add(widget);
        },
        [this, &widget](std::monostate) {
            Window::addWidgetToParent(widget);
        }
    }, m_active_child);
}

/**
 * @brief Opens the province input map.
 *
 * @param filename The full path to the input map to open.
 *
 * @return true if the input was successfully opened, false otherwise
 */
bool MapNormalizer::GUI::MainWindow::importProvinceMap(const Glib::ustring& filename)
{
    // We are checking if the project exists, even though this action should
    //  never be able to be called if theere is no project loaded
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        // First, load the image into memory
        if(BitMap* image = readBMP(filename); image != nullptr) {
            project.getMapProject().setImage(image);
        } else {
            writeError("Failed to read bitmap from ", filename);
            return false;
        }

        const auto* image = project.getMapProject().getImage();

        if(image == nullptr) {
            writeError("Reading bitmap failed.");
            return false;
        }

        auto& worker = GraphicsWorker::getInstance();

        // We now need a new array that the graphics worker can use to display the
        //  rendered image
        auto data_size = image->info_header.width * image->info_header.height * 3;
        unsigned char* graphics_data = new unsigned char[data_size];

        // No memory leak here, since the data will get deleted either at program
        //  exit, or when the next value is loaded
        worker.init(image, graphics_data);
        worker.resetDebugData();
        worker.updateCallback({0, 0, static_cast<uint32_t>(image->info_header.width),
                                     static_cast<uint32_t>(image->info_header.height)});

        // Make sure that the drawing area is sized correctly to draw the entire
        //  image
        m_drawing_area->get_window()->resize(image->info_header.width,
                                             image->info_header.height);

        m_drawing_area->setGraphicsData(graphics_data);
        m_drawing_area->setImage(image);

        ShapeFinder shape_finder(image);

        // Open a progress bar dialog to show the user that we are actually doing
        //  something
        ProgressBarDialog progress_dialog(*this, "Loading...", "", true);
        progress_dialog.setShowText(true);

        // We add the buttons manually here because we need to be able to set their
        //  sensitivity manually
        // TODO: Do we even want a Done button? Or should we just close the
        //  dialog when we're done?
        Gtk::Button* done_button = progress_dialog.add_button("OK", Gtk::RESPONSE_OK);
        done_button->set_sensitive(false);

        Gtk::Button* cancel_button = progress_dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);

        progress_dialog.show_all();

        // Set up the graphics worker callback
        auto last_stage = shape_finder.getStage();
        worker.setWriteCallback([this, &shape_finder, &progress_dialog, &last_stage](const Rectangle& r)
        {
            m_drawing_area->graphicsUpdateCallback(r);

            auto stage = shape_finder.getStage();
            float fstage = static_cast<uint32_t>(stage);
            float fraction = fstage / static_cast<uint32_t>(ShapeFinder::Stage::DONE);

            // TODO: Do we want to calculate a more precise fraction based on
            //   progress during a given stage?

            if(stage != last_stage) {
                last_stage = stage;
                progress_dialog.setText(toString(stage));
            }
            progress_dialog.setFraction(fraction);
        });

        // Start processing the data
        std::thread sf_worker([&shape_finder, done_button, cancel_button]() {
            auto& worker = GraphicsWorker::getInstance();

            auto shapes = shape_finder.findAllShapes();

            auto* image = shape_finder.getImage();

            // Redraw the new image so we can properly show how it should look in the
            //  final output
            // TODO: Do we still want to do this here? Would it not be better to do
            //  it later on?
            if(!prog_opts.quiet)
                setInfoLine("Drawing new graphical image");
            for(auto&& shape : shapes) {
                for(auto&& pixel : shape.pixels) {
                    // Write to both the output data and into the displayed data
                    writeColorTo(image->data, image->info_header.width,
                                 pixel.point.x, pixel.point.y,
                                 shape.unique_color);

                    worker.writeDebugColor(pixel.point.x, pixel.point.y,
                                           shape.unique_color);
                }
            }

            // One final callback update so that the map drawer has the latest
            //  graphical information
            worker.updateCallback({0, 0, static_cast<uint32_t>(image->info_header.width),
                                         static_cast<uint32_t>(image->info_header.height)});

            deleteInfoLine();

            if(!prog_opts.quiet)
                writeStdout("Detected ", std::to_string(shapes.size()), " shapes.");

            // Disable the cancel button (we've already finished), and enable the
            //  done button so that the user can close the box and move on
            done_button->set_sensitive(true);
            cancel_button->set_sensitive(false);
        });

        bool did_estop = false;

        // Run the progress bar dialog
        // If the user cancels the action, then we need to kill sf_worker ASAP
        if(auto response = progress_dialog.run();
                response == Gtk::RESPONSE_DELETE_EVENT ||
                response == Gtk::RESPONSE_CANCEL)
        {
            shape_finder.estop();
            did_estop = true;
        }

        // Wait for the worker to join back up
        sf_worker.join();

        worker.resetWriteCallback();

        // Don't finish importing if we stopped early
        if(did_estop) {
            return true;
        }

        project.getMapProject().setShapeFinder(std::move(shape_finder));
        project.getMapProject().setGraphicsData(graphics_data);

        std::filesystem::path imported(filename);
        std::filesystem::path input_root = project.getInputsRoot();

        if(!std::filesystem::exists(input_root)) {
            std::filesystem::create_directory(input_root);
        }

        // TODO: We should actually do two things here:
        //  1) If filename == input_full_path: do nothing
        //  2) Otherwise ask if they want to overrite/replace the imported province map
        if(auto input_full_path = input_root / INPUT_PROVINCEMAP_FILENAME;
           !std::filesystem::exists(input_full_path))
        {
            std::filesystem::copy_file(imported, input_full_path);
        }
    } else {
        writeError("Unable to complete importing '", filename, "'. Reason: There is no project currently loaded.");
        return false;
    }

    return true;
}

/**
 * @brief Creates a dialog box and, if successful, will create and set a new
 *        project on the Driver
 */
void MapNormalizer::GUI::MainWindow::newProject() {
    NewProjectDialog npd(*this);

    const int result = npd.run();

    Driver::UniqueProject project(nullptr);
    switch(result) {
        case Gtk::RESPONSE_ACCEPT:
            npd.hide();
            project.reset(new Driver::HProject);
            project->setName(npd.getProjectName());

            {
                std::filesystem::path root_path = npd.getProjectPath();
                project->setPath((root_path / project->getName()).replace_extension(PROJ_EXTENSION));
            }

            // Attempt to save just the root project (this will set up the
            //  initial metadata we will need for later)
            if(!project->save(false)) {
                Gtk::MessageDialog dialog(*this, "Failed to save project.", false,
                                          Gtk::MESSAGE_ERROR);
                dialog.run();
                return;
            }

            // TODO: If a project is already open, make sure we close it first

            Driver::getInstance().setProject(std::move(project));

            // We are technically "opening" the project by creating it
            onProjectOpened();
            break;
        case Gtk::RESPONSE_CANCEL:
        default:
            break;
    }
}

void MapNormalizer::GUI::MainWindow::openProject() {
    Driver::UniqueProject project(new Driver::HProject);

    // Allocate this on the stack so that it gets automatically cleaned up
    //  when we finish
    Gtk::FileChooserDialog dialog(*this, "Choose a project file.");
    dialog.set_select_multiple(false);
    // dialog.add_filter(); // TODO: Filter only for supported file types

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Select", Gtk::RESPONSE_ACCEPT);

    const int result = dialog.run();

    switch(result) {
        case Gtk::RESPONSE_ACCEPT:
            dialog.hide(); // Hide ourselves immediately

            project->setPath(dialog.get_filename());
            if(!project->load()) {
                Gtk::MessageDialog dialog("Failed to open file.", false,
                                          Gtk::MESSAGE_ERROR);
                dialog.run();
                return;
            }
            break;
        case Gtk::RESPONSE_CANCEL:
        case Gtk::RESPONSE_DELETE_EVENT:
        default:
            return;
    }

    if(project->getToolVersion() != TOOL_VERSION) {
        std::stringstream message_ss;
        message_ss << "This project was built with a different tool version: '"
                   << project->getToolVersion() << "' != '" << TOOL_VERSION
                   << "\'\nColor data may be generated differently.";

        Gtk::MessageDialog dialog(message_ss.str(), false, Gtk::MESSAGE_WARNING);

        dialog.run();

        // Bring the tool version up to date
        project->setToolVersion(TOOL_VERSION);
    }

    // Set up the drawing area
    const auto& map_project = project->getMapProject();

    m_drawing_area->setGraphicsData(map_project.getGraphicsData());
    m_drawing_area->setImage(map_project.getImage());
    m_drawing_area->queue_draw();

    // We no longer need to own the project, so give it to the Driver
    Driver::getInstance().setProject(std::move(project));

    onProjectOpened();
}

/**
 * @brief Called when a project is opened or created
 */
void MapNormalizer::GUI::MainWindow::onProjectOpened() {
    // Enable all actions that can only be done on an opened project
    getAction("import_provincemap")->set_enabled(true);
    getAction("save")->set_enabled(true);
    getAction("close")->set_enabled(true);
}

/**
 * @brief Called when a project is closed
 */
void MapNormalizer::GUI::MainWindow::onProjectClosed() {
    // Disable all actions that can only be done on an opened project
    getAction("import_provincemap")->set_enabled(false);
    getAction("save")->set_enabled(false);
    getAction("close")->set_enabled(false);
}

/**
 * @brief Saves the currently set Driver project (if one is in fact set)
 */
void MapNormalizer::GUI::MainWindow::saveProject() {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        // Make sure the user is notified if we failed to save the project
        if(!project.save()) {
            Gtk::MessageDialog dialog(*this, "Failed to save file.", false,
                                      Gtk::MESSAGE_ERROR);
            dialog.run();
            return;
        }
    }
}

/**
 * @brief Performs a saveAs operation, asking the user for a new file location
 *        to save to before calling saveProject()
 */
void MapNormalizer::GUI::MainWindow::saveProjectAs(const std::string& dtitle) {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        // Allocate this on the stack so that it gets automatically cleaned up
        //  when we finish
        Gtk::FileChooserDialog dialog(*this, dtitle,
                                      Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_select_multiple(false);

        dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Select", Gtk::RESPONSE_ACCEPT);

        const int result = dialog.run();
        switch(result) {
            case Gtk::RESPONSE_ACCEPT:
                dialog.hide(); // Hide ourselves immediately

                project.setPathAndName(dialog.get_filename());
                saveProject();
                break;
            case Gtk::RESPONSE_CANCEL:
            case Gtk::RESPONSE_DELETE_EVENT:
            default:
                return;
        }
    }
}

