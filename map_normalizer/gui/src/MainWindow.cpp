
#include "MainWindow.h"

#include <iostream>

#include "gtkmm.h"
#include "gtkmm/filechooserdialog.h"

#include "BitMap.h"
#include "Constants.h"
#include "Logger.h"
#include "Util.h" // overloaded

#include "GraphicalDebugger.h"
#include "MapNormalizerApplication.h"
#include "MapDrawingArea.h"

MapNormalizer::GUI::MainWindow::MainWindow(Gtk::Application& application):
    Window(APPLICATION_NAME, application),
    m_image(nullptr),
    m_graphics_data(nullptr)
{
    set_size_request(512, 512);
}

MapNormalizer::GUI::MainWindow::~MainWindow() { }

bool MapNormalizer::GUI::MainWindow::initializeActions() {
    // TODO
#if 0
    add_action("new", []() {
        std::cout << "new" << std::endl;
    });
#endif

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

    add_action("close", [this]() {
        writeStdout("Quitting now!");
        hide();
    });

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

    return true;
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
    auto drawingWindow = addWidget<Gtk::ScrolledWindow>();
    auto drawingArea = new MapDrawingArea();

    // Set pointers on drawingArea so that it knows where to go for image
    //  data that may get updated at any time (also means we don't have to
    //  worry about finding the widget and updating it ourselves)
    drawingArea->setGraphicsDataPtr(&m_graphics_data);
    drawingArea->setImagePtr(&m_image);

    // Place the drawing area in a scrollable window
    drawingWindow->add(*drawingArea);
    drawingWindow->show_all();
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
    m_image = readBMP(filename);

    if(m_image == nullptr) {
        writeError("Reading bitmap failed.");
        return false;
    }

    auto& worker = GraphicsWorker::getInstance();

    auto data_size = m_image->info_header.width * m_image->info_header.height * 3;

    m_graphics_data = new unsigned char[data_size];

    // std::thread graphics_thread;

    // No memory leak here, since the data will get deleted either at program
    //  exit, or when the next value is loaded
    worker.init(m_image, m_graphics_data);
    worker.resetDebugData();

    // TODO: Spawwn a thread and begin loading the data

    return true;
}

