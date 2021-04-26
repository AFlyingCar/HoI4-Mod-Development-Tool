
#include "MainWindow.h"

#include <thread>

#include "gtkmm.h"
#include "gtkmm/filechooserdialog.h"

#include "BitMap.h"
#include "Constants.h"
#include "Logger.h"
#include "Util.h" // overloaded

#include "ShapeFinder2.h" // findAllShapes2

#include "GraphicalDebugger.h"
#include "MapNormalizerApplication.h"
#include "ProgressBarDialog.h"

MapNormalizer::GUI::MainWindow::MainWindow(Gtk::Application& application):
    Window(APPLICATION_NAME, application),
    m_image(nullptr),
    m_graphics_data(nullptr)
{
    set_size_request(512, 512);
}

MapNormalizer::GUI::MainWindow::~MainWindow() { }

bool MapNormalizer::GUI::MainWindow::initializeActions() {
    initializeFileActions();
    initializeEditActions();
    initializeViewActions();
    initializeProjectActions();

    return true;
}

void MapNormalizer::GUI::MainWindow::initializeFileActions() {
    add_action("new", []() {
        // TODO
    });

    add_action("close", [this]() {
        hide();
    });
}

void MapNormalizer::GUI::MainWindow::initializeEditActions() {
}

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
                if(!openInputMap(dialog.get_filename())) {
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

bool MapNormalizer::GUI::MainWindow::initializeWidgets() {
    m_paned = addWidget<Gtk::Paned>();
    
    // Set the minimum size of the pane to 512x512
    m_paned->set_size_request(MINIMUM_WINDOW_W, MINIMUM_WINDOW_H);

    // Make sure that we take up all of the available vertical space
    m_paned->set_vexpand();

    buildViewPane();

    m_active_child = std::monostate{};

    return true;
}

void MapNormalizer::GUI::MainWindow::buildViewPane() {
    m_paned->pack1(*std::get<Gtk::Frame*>(m_active_child = new Gtk::Frame()), true, false);

    // Setup the box+area for the map image to render
    auto drawing_window = addWidget<Gtk::ScrolledWindow>();
    auto drawing_area = m_drawing_area = new MapDrawingArea();

    // Set pointers on drawingArea so that it knows where to go for image
    //  data that may get updated at any time (also means we don't have to
    //  worry about finding the widget and updating it ourselves)
    drawing_area->setGraphicsDataPtr(&m_graphics_data);
    drawing_area->setImagePtr(&m_image);

    // Place the drawing area in a scrollable window
    drawing_window->add(*drawing_area);
    drawing_window->show_all();
}

Gtk::Frame* MapNormalizer::GUI::MainWindow::buildPropertiesPane() {
    Gtk::Frame* properties_frame = new Gtk::Frame();
    m_active_child = properties_frame;

    m_paned->pack2(*properties_frame, false, false);

    // We want to possibly be able to scroll in the properties window
    auto propertiesWindow = addActiveWidget<Gtk::ScrolledWindow>();
    propertiesWindow->set_size_request(MINIMUM_PROPERTIES_PANE_WIDTH, -1);

    // TODO: Add all of the properties stuff
    //   We may want a custom widget that knows what is currently selected?
    addWidget<Gtk::Label>("Properties");

    return properties_frame;
}

Gtk::Orientation MapNormalizer::GUI::MainWindow::getDisplayOrientation() const {
    return Gtk::Orientation::ORIENTATION_VERTICAL;
}

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

bool MapNormalizer::GUI::MainWindow::openInputMap(const Glib::ustring& filename)
{
    // First, load the image into memory
    m_image = readBMP(filename);

    if(m_image == nullptr) {
        writeError("Reading bitmap failed.");
        return false;
    }

    auto& worker = GraphicsWorker::getInstance();

    // We now need a new array that the graphics worker can use to display the
    //  rendered image
    auto data_size = m_image->info_header.width * m_image->info_header.height * 3;
    m_graphics_data = new unsigned char[data_size];

    // No memory leak here, since the data will get deleted either at program
    //  exit, or when the next value is loaded
    worker.init(m_image, m_graphics_data);
    worker.resetDebugData();
    worker.updateCallback({0, 0, static_cast<uint32_t>(m_image->info_header.width),
                                 static_cast<uint32_t>(m_image->info_header.height)});

    // Make sure that the drawing area is sized correctly to draw the entire
    //  image
    m_drawing_area->get_window()->resize(m_image->info_header.width,
                                         m_image->info_header.height);

    ShapeFinder shape_finder(m_image);

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
    std::thread sf_worker([this, &shape_finder, done_button, cancel_button]() {
        auto& worker = GraphicsWorker::getInstance();

        auto shapes = shape_finder.findAllShapes();

        // Redraw the new image so we can properly show how it should look in the
        //  final output
        // TODO: Do we still want to do this here? Would it not be better to do
        //  it later on?
        if(!prog_opts.quiet)
            setInfoLine("Drawing new graphical image");
        for(auto&& shape : shapes) {
            for(auto&& pixel : shape.pixels) {
                // Write to both the output data and into the displayed data
                writeColorTo(m_image->data, m_image->info_header.width,
                             pixel.point.x, pixel.point.y,
                             shape.unique_color);

                worker.writeDebugColor(pixel.point.x, pixel.point.y,
                                       shape.unique_color);
            }
        }

        // One final callback update so that the map drawer has the latest
        //  graphical information
        worker.updateCallback({0, 0, static_cast<uint32_t>(m_image->info_header.width),
                                     static_cast<uint32_t>(m_image->info_header.height)});

        deleteInfoLine();

        if(!prog_opts.quiet)
            writeStdout("Detected ", std::to_string(shapes.size()), " shapes.");

        // Disable the cancel button (we've already finished), and enable the
        //  done button so that the user can close the box and move on
        done_button->set_sensitive(true);
        cancel_button->set_sensitive(false);
    });

    // Run the progress bar dialog
    // If the user cancels the action, then we need to kill sf_worker ASAP
    if(auto response = progress_dialog.run();
            response == Gtk::RESPONSE_DELETE_EVENT ||
            response == Gtk::RESPONSE_CANCEL)
    {
        shape_finder.estop();
    }

    // Wait for the worker to join back up
    sf_worker.join();

    worker.resetWriteCallback();

    return true;
}

