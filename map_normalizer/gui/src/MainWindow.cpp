#include "MainWindow.h"

#include <thread>
#include <sstream>

#include "gtkmm.h"

#include "NativeDialog.h"

#include "BitMap.h"
#include "Constants.h"
#include "Logger.h"
#include "Util.h" // overloaded

#include "ShapeFinder2.h" // ShapeFinder

#include "GraphicalDebugger.h"
#include "InterruptableScrolledWindow.h"
#include "MapNormalizerApplication.h"
#include "ProgressBarDialog.h"
#include "NewProjectDialog.h"
#include "Driver.h"
#include "MapDrawingArea.h"
#include "SelectionManager.h"

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
    initializeHelpActions();

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
                WRITE_DEBUG("SaveProjectAs(", project.getPath(), ")");
                saveProjectAs();
            } else {
                WRITE_DEBUG("SaveProject(", project.getPath(), ")");
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
    auto properties_action = add_action_bool("properties", [this]() {
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
    properties_action->set_enabled(false);

    add_action("log_window", [this]() {
        if(m_log_viewer_window == nullptr) {
            m_log_viewer_window.reset(new LogViewerWindow());
            m_log_viewer_window->show_all();
        } else {
            m_log_viewer_window->present();
        }
    });

    // Switch Renderers actions
    {
        auto usegl_action = add_action_bool("switch_renderers.usegl", [this]() {
            // Change us to be enabled
            auto self = lookup_action("switch_renderers.usegl");
            self->change_state(true);

            // Change the other actions to be disabled
            auto usecairo = lookup_action("switch_renderers.usecairo");
            usecairo->change_state(false);

            // Swap over to use OpenGL
            m_drawing_area->hide();
            m_drawing_area = m_gl_drawing_area;

            // Replace the widget that is gettiing rendered
            // m_drawing_box->remove(); // TODO
            m_drawing_box->pack_start(*m_gl_drawing_area, Gtk::PACK_SHRINK);

            m_drawing_area->show();

            // Make sure we update the drawing area with any new data it may have
            //  missed
            m_drawing_area->setMapData(m_cairo_drawing_area->getMapData());
            m_drawing_area->queueDraw();
        });

        auto usecairo_action = add_action_bool("switch_renderers.usecairo", [this]()
        {
            // Change us to be enabled
            auto self = lookup_action("switch_renderers.usecairo");
            self->change_state(true);

            // Change the other actions to be disabled
            auto usegl = lookup_action("switch_renderers.usegl");
            usegl->change_state(false);

            // Swap over to use Cairo
            m_drawing_area->hide();
            m_drawing_area = m_cairo_drawing_area;

            // Replace the widget that is gettiing rendered
            // m_drawing_box->remove(); // TODO
            m_drawing_box->pack_start(*m_cairo_drawing_area, Gtk::PACK_SHRINK);

            m_drawing_area->show();

            // Make sure we update the drawing area with any new data it may have
            //  missed
            m_drawing_area->setMapData(m_gl_drawing_area->getMapData());

            // Special Cairo functions we need to call
            m_cairo_drawing_area->rebuildImageCache();

            m_drawing_area->queueDraw();
        });

#if MN_DEFAULT_RENDERING_TO_GL
        usegl_action ->change_state(true);
#else
        usecairo_action->change_state(true);
#endif
    }
}

/**
 * @brief Initializes every action in the Project menu
 */
void MapNormalizer::GUI::MainWindow::initializeProjectActions() {
    auto ipm_action = add_action("import_provincemap", [this]() {
        // Allocate this on the stack so that it gets automatically cleaned up
        //  when we finish
        NativeDialog::FileDialog dialog("Choose an input image file",
                                        NativeDialog::FileDialog::SELECT_FILE);
        // dialog.setDefaultPath() // TODO: Start in the installation directory/Documents
        std::string path;
        dialog.addFilter("Province Image Files", "bmp")
              .addFilter("All files", "*")
              .setAllowsMultipleSelection(false)
              .setDecideHandler([&path](const NativeDialog::Dialog& dialog) {
                    auto& fdlg = dynamic_cast<const NativeDialog::FileDialog&>(dialog);
                    path = fdlg.selectedPathes().front();
              }).show();

          if(!path.empty() && !importProvinceMap(path)) {
              Gtk::MessageDialog err_diag("Failed to open file.",
                                          false, Gtk::MESSAGE_ERROR);
              err_diag.run();
          }
    });

    // This action should be disabled by default, until a project gets opened
    ipm_action->set_enabled(false);
}

/**
 * @brief Initializes every action in the Help menu
 */
void MapNormalizer::GUI::MainWindow::initializeHelpActions() {
    add_action("about", []() {
        Gtk::AboutDialog dialog;

        // Credits
        dialog.set_authors({ "Tyler Robbins" });
        dialog.add_credit_section("Libraries Used:", {
            "gtkmm", "nlohmann::json", "nlohmann::fifo_map", "GLEW", "OpenGL",
            "GLM", "Native Dialogs", "gtest"
        });
        dialog.set_artists({"Lapshaman"});
        // dialog.set_translators({});

        // Program information
        dialog.set_program_name(APPLICATION_NAME);

        dialog.set_version(TOOL_VERSION.str());

        dialog.set_license(TOOL_LICENSE);
        dialog.set_wrap_license(true);

        dialog.set_website_label("Source code");
        dialog.set_website(SOURCE_LOCATION);

        dialog.set_logo(Gdk::Pixbuf::create_from_resource("/com/aflyingcar/MapNormalizerTools/textures/logo.png"));

        dialog.run();
    });

    // TODO: Link to the wiki once it is written
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

    // If the escape key is pressed, deselect the current province
    signal_key_press_event().connect([](GdkEventKey* event) {
        if(event->keyval == GDK_KEY_Escape) {
            SelectionManager::getInstance().clearProvinceSelection();
        }

        return false;
    });

    return true;
}

bool MapNormalizer::GUI::MainWindow::initializeFinal() {
    set_icon(Gdk::Pixbuf::create_from_resource("/com/aflyingcar/MapNormalizerTools/textures/logo.png"));

    initializeCallbacks();

    return true;
}

void MapNormalizer::GUI::MainWindow::initializeCallbacks() {
    SelectionManager::getInstance().setOnSelectProvinceCallback(
        [this](uint32_t prov_id, SelectionManager::Action action)
        {
            auto& map_project = Driver::getInstance().getProject()->get().getMapProject();

            switch(action) {
                case SelectionManager::Action::SET:
                case SelectionManager::Action::ADD:
                    // If the label is a valid province, then go ahead and mark it as
                    //  selected everywhere that needs it to be marked as such
                    if(map_project.isValidProvinceLabel(prov_id)) {
                        // The selected province
                        auto* province = &map_project.getProvinceForLabel(prov_id);

                        // TODO: We should really move preview data out of MapProject
                        //   too
                        auto preview_data = map_project.getPreviewData(province);

                        if(action == SelectionManager::Action::SET) {
                            if(m_province_properties_pane != nullptr) {
                                m_province_properties_pane->setProvince(province, preview_data);
                            }

                            m_drawing_area->setSelection({preview_data, province->bounding_box, province->id});
                        } else {
                            if(m_province_properties_pane != nullptr) {
                                // This will be true if there are already any selections in the
                                //  list. In other words, the first selection should always
                                //  populate the properties pane, but subsequent selections
                                //  should not.
                                const auto& selected_labels = SelectionManager::getInstance().getSelectedProvinceLabels();
                                bool has_selections_already = !selected_labels.empty();

                                m_province_properties_pane->setProvince(province, preview_data, has_selections_already);
                            }

                            m_drawing_area->addSelection({preview_data, province->bounding_box, province->id});
                        }

                        m_drawing_area->queueDraw();
                    }
                    break;
                case SelectionManager::Action::REMOVE:
                    // Only remove the province from the properties pane if
                    //  the province there is the same one we are removing
                    if(m_province_properties_pane != nullptr &&
                       m_province_properties_pane->getProvince() != nullptr &&
                       m_province_properties_pane->getProvince()->id == prov_id)
                    {
                        // TODO: We should really be changing to the next one in
                        //   the selection if we have one selected.
                        ProvincePreviewDrawingArea::DataPtr null_data; // Do not construct
                        m_province_properties_pane->setProvince(nullptr, null_data);
                    }
                    m_drawing_area->removeSelection({nullptr, {}, prov_id});
                    m_drawing_area->queueDraw();
                    break;
                case SelectionManager::Action::CLEAR:
                    if(m_province_properties_pane != nullptr) {
                        ProvincePreviewDrawingArea::DataPtr null_data; // Do not construct
                        m_province_properties_pane->setProvince(nullptr, null_data);
                    }

                    m_state_properties_pane->setState(nullptr);

                    m_drawing_area->setSelection();
                    m_drawing_area->queueDraw();

                    break;
            }
        });

    SelectionManager::getInstance().setOnSelectStateCallback(
        [this](StateID state_id, SelectionManager::Action action)
        {
            auto& map_project = Driver::getInstance().getProject()->get().getMapProject();

            switch(action) {
                case SelectionManager::Action::SET:
                case SelectionManager::Action::ADD:
                    // TODO: we can't use the above is_already_selected or
                    //   has_selections_already variables since those are
                    //   referring to provinces, so we need to do similar
                    //   calculations again but for states
                    if(m_state_properties_pane != nullptr) {
                        auto* state = &map_project.getStateForID(state_id);
                        m_state_properties_pane->setState(state);
                    }

                    // TODO: Update state drawing area once we have that
                    //   Use different behavior for add/select
                    break;
                case SelectionManager::Action::REMOVE:
                    // TODO: Update state drawing area once we have that for removal
                case SelectionManager::Action::CLEAR:
                    if(m_state_properties_pane != nullptr) {
                        m_state_properties_pane->setState(nullptr);
                    }
                    break;
            }
        });
}

auto MapNormalizer::GUI::MainWindow::getLogViewerWindow()
    -> OptionalReference<LogViewerWindow>
{
    if(m_log_viewer_window == nullptr) {
        return std::nullopt;
    } else {
        return *m_log_viewer_window;
    }
}

/**
 * @brief Builds the view pane, which is where the map gets rendered to.
 */
void MapNormalizer::GUI::MainWindow::buildViewPane() {
    m_paned->pack1(*std::get<Gtk::Frame*>(m_active_child = new Gtk::Frame()), true, false);

    // Setup the box+area for the map image to render
    auto drawing_window = nameWidget("drawing_window",
                                     addWidget<InterruptableScrolledWindow>());

    auto setup_drawing_area = [](std::shared_ptr<IMapDrawingAreaBase> drawing_area)
    {
        drawing_area->setOnProvinceSelectCallback([](uint32_t x, uint32_t y) {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
                auto& project = opt_project->get();
                auto& map_project = project.getMapProject();

                auto map_data = project.getMapProject().getMapData();
                auto lmatrix = project.getMapProject().getLabelMatrix();

                // If the click happens outside of the bounds of the image, then
                //   deselect the province
                if(x > map_data->getWidth() || y > map_data->getHeight()) {
                    SelectionManager::getInstance().clearProvinceSelection();

                    return;
                }

                // Get the label for the pixel that got clicked on
                auto label = lmatrix[xyToIndex(map_data->getWidth(), x, y)];

                WRITE_DEBUG("Selecting province with ID ", label);
                SelectionManager::getInstance().selectProvince(label - 1);

                // If this is a valid province, then select the state that it is
                //  a part of (if it is a part of one at all, that is)
                if(map_project.isValidProvinceLabel(label - 1)) {
                    auto& prov = map_project.getProvinceForLabel(label -1);

                    // Don't bother checking for if it's valid or not, as
                    //  MapProject will do that for us
                    SelectionManager::getInstance().selectState(prov.state);
                } else {
                    SelectionManager::getInstance().clearStateSelection();
                }
            }
        });

        drawing_area->setOnMultiProvinceSelectionCallback([](uint32_t x, uint32_t y)
        {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
                auto& project = opt_project->get();
                auto& map_project = project.getMapProject();

                auto map_data = map_project.getMapData();
                auto lmatrix = map_project.getLabelMatrix();

                // Multiselect out of bounds will simply not add to the selections
                if(x > map_data->getWidth() || y > map_data->getHeight()) {
                    return;
                }

                auto label = lmatrix[xyToIndex(map_data->getWidth(), x, y)];

                // Go over the list of already selected provinces and check if
                //  we have clicked on one that is _already_ selected
                const auto& selected_labels = SelectionManager::getInstance().getSelectedProvinceLabels();
                bool is_already_selected = selected_labels.count(label - 1);

                // Perform a check here to make sure we don't go over the maximum
                //   number of selectable provinces if we are not deselecting one
                if(!is_already_selected && selected_labels.size() == MAX_SELECTED_PROVINCES)
                {
                    WRITE_WARN("Maximum number of provinces selected! Cannot select more than ",
                               MAX_SELECTED_PROVINCES, " at once!");
                    return;
                }

                // Do not mark this province as selected if we are deselecting it
                if(is_already_selected) {
                    SelectionManager::getInstance().removeProvinceSelection(label - 1);
                } else {
                    SelectionManager::getInstance().addProvinceSelection(label - 1);
                }

                // If this is a valid province, then select the state that it is
                //  a part of (if it is a part of one at all, that is)
                if(map_project.isValidProvinceLabel(label - 1)) {
                    auto& prov = map_project.getProvinceForLabel(label -1);

                    // Don't bother checking for if it's valid or not, as
                    //  MapProject will do that for us
                    SelectionManager::getInstance().addStateSelection(prov.state);
                } else {
                    SelectionManager::getInstance().removeStateSelection(label - 1);
                }
            }
        });
    };

    // Setup each drawing area type
    m_gl_drawing_area.reset(new GL::MapDrawingArea);
    setup_drawing_area(m_gl_drawing_area);
    m_cairo_drawing_area.reset(new MapDrawingArea);
    setup_drawing_area(m_cairo_drawing_area);

    // Setup initially enabled drawing area
    auto drawing_area = m_drawing_area =
#if MN_DEFAULT_RENDERING_TO_GL
        m_gl_drawing_area;
#else
        m_cairo_drawing_area;
#endif

    // Set up a signal callback to zoom in and out when performing CTRL+ScrollWhell
    drawing_window->signalOnScroll().connect([drawing_area](GdkEventScroll* event)
    {
        if(event->state & GDK_CONTROL_MASK) {
            switch(event->direction) {
                case GDK_SCROLL_UP:
                    drawing_area->zoom(MapDrawingArea::ZoomDirection::IN);
                    break;
                case GDK_SCROLL_DOWN:
                    drawing_area->zoom(MapDrawingArea::ZoomDirection::OUT);
                    break;
                case GDK_SCROLL_SMOOTH:
                    drawing_area->zoom(-event->delta_y * ZOOM_FACTOR);
                    break;
                default: // We don't care about _LEFT or _RIGHT
                    break;
            }
            return true;
        }

        return false;
    });

    // Set up a signal callback to zoom in and out when pressing NumpadADD and NumpadSUB
    // CTRL+r will reset zoom level
    drawing_window->add_events(Gdk::KEY_PRESS_MASK);
    drawing_window->signal_key_press_event().connect([drawing_area](GdkEventKey* event)
    {
        switch(event->keyval) {
            case GDK_KEY_KP_Add:
                drawing_area->zoom(MapDrawingArea::ZoomDirection::IN);
                break;
            case GDK_KEY_KP_Subtract:
                drawing_area->zoom(MapDrawingArea::ZoomDirection::OUT);
                break;
            case GDK_KEY_r:
                if(event->state & GDK_CONTROL_MASK) {
                    drawing_area->zoom(MapDrawingArea::ZoomDirection::RESET);
                }
                break;
        }

        return false;
    });

    // Place the drawing area in a scrollable window
    // drawing_window->add(*drawing_area->self());
    m_drawing_box.reset(new Gtk::Box());
    m_drawing_box->pack_start(*drawing_area->self(), Gtk::PACK_SHRINK);
    drawing_window->add(*m_drawing_box);
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

    // Province Tab
    auto properties_tab = addActiveWidget<Gtk::Notebook>();

    {
        m_province_properties_pane.reset(new ProvincePropertiesPane);
        m_province_properties_pane->init();
        properties_tab->append_page(m_province_properties_pane->getParent(), "Province");
    }

    // State Tab
    {
        m_state_properties_pane.reset(new StatePropertiesPane);
        m_state_properties_pane->init();

        properties_tab->append_page(m_state_properties_pane->getParent(), "State");
    }

    m_paned->property_position().signal_changed().connect([this]() {
        if(m_province_properties_pane) {
            m_province_properties_pane->onResize();
        }

        if(m_state_properties_pane) {
            m_state_properties_pane->onResize();
        }
    });

    m_active_child = std::monostate();

    // Finish extra setup in case we have a project loaded
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();
        auto& map_project = project.getMapProject();

        // Provinces Tab
        if(auto selected = SelectionManager::getInstance().getSelectedProvinces(); !selected.empty())
        {
            auto* province = &selected.begin()->get();
            auto label = province->id;
            m_province_properties_pane->setProvince(province,
                                                    map_project.getPreviewData(label),
                                                    selected.size() > 1);
        }
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
        BitMap* image = nullptr;
        if(image = readBMP(std::string(filename)); image == nullptr) {
            WRITE_ERROR("Failed to read bitmap from ", filename);
            return false;
        }

        if(image == nullptr) {
            WRITE_ERROR("Reading bitmap failed.");
            return false;
        }

        auto& worker = GraphicsWorker::getInstance();

        // We now need a new array that the graphics worker can use to display the
        //  rendered image
        std::shared_ptr<MapData> map_data(new MapData(image->info_header.width,
                                                      image->info_header.height));

        // No memory leak here, since the data will get deleted either at program
        //  exit, or when the next value is loaded
        worker.init(map_data);
        worker.resetDebugData();
        worker.updateCallback({0, 0, static_cast<uint32_t>(image->info_header.width),
                                     static_cast<uint32_t>(image->info_header.height)});

        // Make sure that the drawing area is sized correctly to draw the entire
        //  image
        m_drawing_area->getWindow()->resize(image->info_header.width,
                                            image->info_header.height);

        m_drawing_area->setMapData(map_data);

        ShapeFinder shape_finder(image, GraphicsWorker::getInstance());

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

        // TODO: This threading stuff is dangerous because GTK is not actually
        //  thread safe. We should make sure to do GTK things to ensure that
        //  nothing causes thread race conditions.

        // Start processing the data
        std::thread sf_worker([&shape_finder, map_data, done_button, cancel_button]()
        {
            auto& worker = GraphicsWorker::getInstance();

            auto shapes = shape_finder.findAllShapes();

            auto* image = shape_finder.getImage();

            auto prov_ptr = map_data->getProvinces().lock();

            // Redraw the new image so we can properly show how it should look in the
            //  final output
            // TODO: Do we still want to do this here? Would it not be better to do
            //  it later on?
            for(auto&& shape : shapes) {
                for(auto&& pixel : shape.pixels) {
                    // Write to both the output data and into the displayed data
                    writeColorTo(prov_ptr.get(), image->info_header.width,
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

            WRITE_INFO("Detected ", shapes.size(), " shapes.");

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

        WRITE_DEBUG("Waiting for ShapeFinder worker to rejoin.");
        sf_worker.join();

        // Note: We reset the zoom here so that we can ensure that the drawing
        //  area actually updates the image.
        // TODO: Why do I have to do this? What about this PR has caused this
        //  to suddenly be required?
        m_drawing_area->resetZoom();

        worker.resetWriteCallback();

        // Don't finish importing if we stopped early
        if(did_estop) {
            return true;
        }

        WRITE_DEBUG("Assigning the found data to the map project.");
        project.getMapProject().setShapeFinder(std::move(shape_finder));
        project.getMapProject().setGraphicsData(map_data);

        // We need to re-assign the data into the drawing area to update the
        //   texture on the drawing area
        // TODO: For OpenGL we should probably use a Pixel Buffer Object (PBO)
        //   so that the texture data doesn't have to be uploaded twice and
        //   instead can be drawn as it is getting generated
        WRITE_DEBUG("Assigning the found data into the drawing area.");
        m_drawing_area->setMapData(map_data);

        std::filesystem::path imported{std::string(filename)};
        std::filesystem::path input_root = project.getInputsRoot();

        WRITE_DEBUG("Copying the imported map into ", input_root);
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
        WRITE_ERROR("Unable to complete importing '", filename, "'. Reason: There is no project currently loaded.");
        return false;
    }

    WRITE_INFO("Import Finished!");

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
    std::string path;

    NativeDialog::FileDialog dialog("Choose a project file.",
                                    NativeDialog::FileDialog::SELECT_FILE);
    // dialog.setDefaultPath() // TODO: Start in the installation directory/Documents
    dialog.addFilter("Project Files", "hoi4proj")
          .addFilter("All files", "*")
          .setAllowsMultipleSelection(false)
          .setDecideHandler([&path](const NativeDialog::Dialog& dialog) {
                auto& fdlg = dynamic_cast<const NativeDialog::FileDialog&>(dialog);
                path = fdlg.selectedPathes().front();
          }).show();

    if(!path.empty()) {
        project->setPath(path);
        if(!project->load()) {
            Gtk::MessageDialog err_diag("Failed to open file.", false,
                                        Gtk::MESSAGE_ERROR);
            err_diag.run();
            return;
        }
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

    m_drawing_area->setMapData(map_project.getMapData());
    m_drawing_area->queueDraw();

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
    getAction("properties")->set_enabled(true);
}

/**
 * @brief Called when a project is closed
 */
void MapNormalizer::GUI::MainWindow::onProjectClosed() {
    // Have the drawing area forget the data it was set to render
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        opt_project->get().getMapProject().getMapData()->close();
    }
    m_drawing_area->queueDraw();

    // Disable all actions that can only be done on an opened project
    getAction("import_provincemap")->set_enabled(false);
    getAction("save")->set_enabled(false);
    getAction("close")->set_enabled(false);
    getAction("properties")->set_enabled(false);

    if(m_province_properties_pane != nullptr) {
        ProvincePreviewDrawingArea::DataPtr null_data; // Do not construct
        m_province_properties_pane->setProvince(nullptr, null_data);

        m_drawing_area->setSelection();
        m_drawing_area->queueDraw();
    }
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
        NativeDialog::FileDialog dialog(dtitle,
                                        NativeDialog::FileDialog::SELECT_FILE |
                                        NativeDialog::FileDialog::SELECT_TO_SAVE);
        // dialog.setDefaultPath() // TODO: Start in the installation directory/Documents
        dialog.addFilter("Project Files", "hoi4proj")
              .addFilter("All files", "*")
              .setAllowsMultipleSelection(false)
              .setDecideHandler([this, &project](const NativeDialog::Dialog& dialog)
              {
                    auto& fdlg = dynamic_cast<const NativeDialog::FileDialog&>(dialog);
                    auto&& paths = fdlg.selectedPathes();

                    project.setPathAndName(paths.front());
                    saveProject();
              }).show();
    }
}

