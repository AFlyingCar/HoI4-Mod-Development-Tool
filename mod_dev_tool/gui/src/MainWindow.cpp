#include "MainWindow.h"

#include <thread>
#include <sstream>

#include <libintl.h>

#include "gtkmm.h"

#include "NativeDialog.h"

#include "BitMap.h"
#include "Constants.h"
#include "Logger.h"
#include "Util.h" // overloaded
#include "Options.h"
#include "Preferences.h"
#include "StatusCodes.h"

#include "ShapeFinder2.h" // ShapeFinder

#include "ActionManager.h"

#include "GraphicalDebugger.h"
#include "Application.h"
#include "ProgressBarDialog.h"
#include "NewProjectDialog.h"
#include "Driver.h"
#include "SelectionManager.h"

#include "Item.h"

#include "NodeKeyNames.h"

/**
 * @brief Constructs the main window.
 *
 * @param application The application that this window is a part of
 */
HMDT::GUI::MainWindow::MainWindow(Gtk::Application& application):
    BaseMainWindow(APPLICATION_NAME, application),
    MainWindowDrawingAreaPart()
{
    set_size_request(512, 512);
}

HMDT::GUI::MainWindow::~MainWindow() { }

/**
 * @brief Initializes every action for the menubar
 *
 * @return true
 */
bool HMDT::GUI::MainWindow::initializeActions() {
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
void HMDT::GUI::MainWindow::initializeFileActions() {
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

    auto add_item_action = add_action("add_item", [this]() {
        if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
            if(m_add_file_window == nullptr) {
                m_add_file_window.reset(new AddFileWindow(*this));
                m_add_file_window->show_all();
            } else {
                m_add_file_window->present();
            }
        }
    });
    add_item_action->set_enabled(false);
}

/**
 * @brief Initializes every action in the Edit menu
 */
void HMDT::GUI::MainWindow::initializeEditActions() {
    add_action("undo", []() {
        if(!Action::ActionManager::getInstance().undoAction()) {
            WRITE_WARN("Failed to undo action.");
        }
    });

    add_action("redo", []() {
        if(!Action::ActionManager::getInstance().redoAction()) {
            WRITE_WARN("Failed to redo action.");
        }
    });
}

/**
 * @brief Initializes every action in the View menu
 */
void HMDT::GUI::MainWindow::initializeViewActions() {
    add_action("log_window", [this]() {
        if(m_log_viewer_window == nullptr) {
            m_log_viewer_window.reset(new LogViewerWindow());
            m_log_viewer_window->show_all();
        } else {
            m_log_viewer_window->present();
        }
    });

    // Switch Renderers actions
#if 0
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

        usegl_action->change_state(true);
    }
#endif

    // Switch Views actions
    {
        auto provinceview_action = add_action_bool("switch_views.province", [this]()
        {
            // Change us to be enabled
            auto self = lookup_action("switch_views.province");
            self->change_state(true);

            // Change the state view to be disabled
            auto state_option = lookup_action("switch_views.state");
            state_option->change_state(false);

            switchRenderingView(IMapDrawingAreaBase::ViewingMode::PROVINCE_VIEW);
        });

        auto stateview_action = add_action_bool("switch_views.state", [this]() {
            // Change us to be enabled
            auto self = lookup_action("switch_views.state");
            self->change_state(true);

            // Change the province view to be disabled
            auto province_option = lookup_action("switch_views.province");
            province_option->change_state(false);

            switchRenderingView(IMapDrawingAreaBase::ViewingMode::STATES_VIEW);
        });

        provinceview_action->change_state(true);
    }

    // Debug actions
    {
        auto render_adjacencies_action = add_action_bool("debug.render_adjacencies", [this]()
        {
            // First lookup our current state
            auto self = lookupAction<Gio::SimpleAction>("debug.render_adjacencies");
            bool state;
            self->get_state<bool>(state);

            if(state) {
                WRITE_INFO("Disabling adjacency rendering!");
            } else {
                WRITE_INFO("Enabling adjacency rendering!");
            }

            // Now change the relevant values + ourself depending on what the
            //   current state is
            setShouldDrawAdjacencies(!state);
            self->change_state(!state);
        });
        render_adjacencies_action->change_state(false);
        render_adjacencies_action->set_enabled(prog_opts.debug);
    }
}

/**
 * @brief Switches the rendering view
 *
 * @param new_mode The new rendering view to switch to.
 *
 * @return The previous rendering view mode
 */
auto HMDT::GUI::MainWindow::switchRenderingView(IMapDrawingAreaBase::ViewingMode new_mode) noexcept
    -> IMapDrawingAreaBase::ViewingMode
{
    auto prev_mode = m_drawing_area->setViewingMode(new_mode);
    WRITE_DEBUG("Switched from rendering view ", prev_mode, " to ", new_mode);
    return prev_mode;
}

/**
 * @brief Initializes every action in the Project menu
 */
void HMDT::GUI::MainWindow::initializeProjectActions() {
    {
        auto recalc_coasts_action = add_action("recalc_coasts", [this]() {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project)
            {
                auto& map_project = opt_project->get().getMapProject();

                map_project.calculateCoastalProvinces();

                // Now make sure that we update the properties pane info
                getProvincePropertiesPane().updateProperties(SelectionManager::getInstance().getSelectedProvinceCount() > 1);
            } else {
                WRITE_ERROR("Cannot recalculate coasts without a valid project loaded.");
            }
        });

        // This action should be disabled by default, until a project gets opened
        recalc_coasts_action->set_enabled(false);
    }

    {
        auto export_project_action = add_action("export_project", [this]() {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project)
            {
                auto export_path = opt_project->get().getExportRoot();

                if(!std::filesystem::exists(export_path)) {
                    WRITE_DEBUG("ExportProjectAs(", export_path, ")");
                    exportProjectAs();
                } else {
                    WRITE_DEBUG("ExportProject(", export_path, ")");
                    exportProject();
                }
            } else {
                WRITE_ERROR("No project is loaded, unable to export.");
            }
        });
        export_project_action->set_enabled(false);

        auto export_project_as_action = add_action("export_project_as", [this]()
        {
            exportProjectAs();
        });
        export_project_as_action->set_enabled(false);
    }

    {
        auto generate_template_rivers_action = add_action("generate_template_rivers",
        [this]()
        {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project)
            {
                auto input_path = opt_project->get().getInputsRoot();
                auto river_path = input_path / RIVERS_FILENAME;

                auto& rivers_project = opt_project->get().getMapProject().getRiversProject();

                // Generate a template "input" image, write it to the input/
                //   folder
                WRITE_DEBUG("Writing template Rivers map to ", input_path);
                auto res = rivers_project.writeTemplate(river_path);
                WRITE_IF_ERROR(res);

                // Load that image back into the RiversProject
                WRITE_DEBUG("Loading template Rivers map back into memory.");
                res = rivers_project.loadFile(river_path);
                WRITE_IF_ERROR(res);

                {
                    std::stringstream ss;
                    ss << "<b>"
                       << gettext("Successfully generated template river map.")
                       << "</b>\n\n"
                       << river_path.generic_string();
                    Gtk::MessageDialog dialog(*this, ss.str(), true,
                                              Gtk::MESSAGE_INFO);
                    dialog.run();
                }
            } else {
                WRITE_ERROR("No project is loaded, unable to generate template river map.");
            }
        });
        generate_template_rivers_action->set_enabled(false);
    }
}

/**
 * @brief Initializes every action in the Help menu
 */
void HMDT::GUI::MainWindow::initializeHelpActions() {
    auto toggle_debug_action = add_action_bool("toggle_debug", [this]() {
        prog_opts.debug = !prog_opts.debug;

        // Find ourselves and change our state
        {
            auto self = lookup_action("toggle_debug");
            self->change_state(prog_opts.debug);
        }

        // Look for each menu item that needs to have its sensitivity toggled
        {
            auto action = lookupAction<Gio::SimpleAction>("debug.render_adjacencies");
            action->set_enabled(prog_opts.debug);
        }
    });
    toggle_debug_action->change_state(prog_opts.debug);
    toggle_debug_action->set_enabled(true);

    add_action("about", []() {
        Gtk::AboutDialog dialog;

        // Credits
        dialog.set_authors({ "Tyler Robbins" });
        dialog.add_credit_section(gettext("Libraries Used:"), {
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

        dialog.set_website_label(gettext("Source code"));
        dialog.set_website(SOURCE_LOCATION);

        dialog.set_logo(Gdk::Pixbuf::create_from_resource("/com/aflyingcar/HoI4ModDevelopmentTool/textures/logo.png"));

        dialog.run();
    });

    add_action("config_editor", [this]() {
        if(m_config_editor_window == nullptr) {
            m_config_editor_window.reset(new ConfigEditorWindow());
            m_config_editor_window->show_all();
        } else {
            m_config_editor_window->present();
        }
    });

    // TODO: Link to the wiki once it is written
}

/**
 * @brief Initializes every widget used by this window
 *
 * @return true
 */
bool HMDT::GUI::MainWindow::initializeWidgets() {
    buildToolbar();

    m_left_pane = manage(new Gtk::Paned());
    m_left_pane->set_position(DEFAULT_FILE_TREE_POSITION);

    m_paned = addWidget<Gtk::Paned>();
    
    // Set the minimum size of the pane to 512x512
    m_paned->set_size_request(MINIMUM_WINDOW_W, MINIMUM_WINDOW_H);

    // Make sure that we take up all of the available vertical space
    m_paned->set_vexpand();

    m_paned->pack1(*m_left_pane);

    buildFileTree(m_left_pane);

    // The view pane will always be loaded, so load it now
    buildViewPane();

    // Build the properties pane now, even though it may not be visible
    buildPropertiesPane(m_paned);

    setActiveChild();

    // If the escape key is pressed, deselect the current province
    signal_key_press_event().connect([](GdkEventKey* event) {
        if(event->keyval == GDK_KEY_Escape) {
            SelectionManager::getInstance().clearProvinceSelection();
        }

        return false;
    });

    return true;
}

bool HMDT::GUI::MainWindow::initializeFinal() {
    set_icon(Gdk::Pixbuf::create_from_resource("/com/aflyingcar/HoI4ModDevelopmentTool/textures/logo.png"));

    initializeCallbacks();

    Preferences::getInstance().getPreferenceValue<bool>("Debug.Logging.openLogWindowOnLaunch")
        .andThen([this](bool open_log_window_on_launch) {
            if(open_log_window_on_launch) {
                auto open_log_window_action = lookup_action("log_window");

                open_log_window_action->activate();
            }
        });


    Preferences::getInstance().getPreferenceValue<bool>("Debug.Graphics.renderAdjacenciesByDefault")
        .andThen([this](bool render_adjacencies_by_default) {
            // Make sure we actually activate it if the default value is 'true'
            if(render_adjacencies_by_default) {
                auto render_adjacencies_action = lookup_action("debug.render_adjacencies");

                render_adjacencies_action->activate();
            }
        });

    return true;
}

void HMDT::GUI::MainWindow::initializeCallbacks() {
    // SelectionManager callbacks
    {
        SelectionManager::getInstance().setOnSelectProvinceCallback(
            [this](const UUID& prov_id,
                   SelectionManager::Action action,
                   SelectionManager::OptCallbackData data)
            {
                WRITE_DEBUG("Select province ", prov_id, ", action=", (int)action);

                auto& map_project = Driver::getInstance().getProject()->get().getMapProject();

                // All possible expected structures that might be held within data
                MainWindowFileTreePart::OnSelectNodeData* filetree_data = nullptr;

                // Pull out the correct data field
                if(data.has_value()) {
                    if(data->type() == typeid(MainWindowFileTreePart::OnSelectNodeData))
                    {
                        filetree_data = std::any_cast<MainWindowFileTreePart::OnSelectNodeData>(&(*data));
                    } else {
                        WRITE_WARN("Got unexpected type in callback data: ",
                                   data->type().name());
                    }
                }

                // Global settings that might be set by one or more data structures
                bool skip_select_in_filetree = false;
                std::optional<Project::Hierarchy::Key> select_in_tree_override = std::nullopt;

                // Set global settings based on provided data
                if(filetree_data != nullptr) {
                    skip_select_in_filetree = filetree_data->skip_select_in_tree;
                    select_in_tree_override = filetree_data->select_in_tree_override;
                }

                switch(action) {
                    case SelectionManager::Action::SET:
                    case SelectionManager::Action::ADD:
                        // If the label is a valid province, then go ahead and mark it as
                        //  selected everywhere that needs it to be marked as such
                        if(map_project.getProvinceProject().isValidProvinceID(prov_id)) {
                            // The selected province
                            auto& province_project = map_project.getProvinceProject();

                            // Get all child provinces and parent provinces and
                            //   add those to the selection too
                            // Also make sure that we also only do "setProvince"
                            //   for the root parent of the entire hierarchy

                            auto&& merged_provinces = province_project.getMergedProvinces(prov_id);
                            if(merged_provinces.empty()) {
                                WRITE_ERROR("Got 0 elements from getMergedProvinces! Cannot select.");
                            } else {
                                WRITE_DEBUG("Selecting ", merged_provinces.size(), " provinces at once.");

                                if(action == SelectionManager::Action::SET) {
                                    m_drawing_area->setSelection();
                                }

                                auto maybe_root = province_project.getRootProvinceParent(prov_id);
                                ProvinceID root_id;
                                if(IS_FAILURE(maybe_root)) {
                                    WRITE_ERROR("Failed with error ",
                                                maybe_root.error(),
                                                " to get root province parent for ",
                                                prov_id,
                                                ". Continuing with legacy behavior.");
                                    // Assume the root is 'prov_id'
                                    // This isn't _quite_ legacy behavior, but
                                    //   close enough
                                    root_id = prov_id;
                                } else {
                                    root_id = maybe_root->get().id;
                                }

                                // List of keys for each merged province
                                std::vector<Project::Hierarchy::Key> keys;

                                // Go over every province in the merged
                                //   provinces and add them to the selection
                                for(auto&& merged_prov : merged_provinces) {
                                    if(merged_prov != prov_id &&
                                       !SelectionManager::getInstance().isProvinceSelected(merged_prov))
                                    {
                                        SelectionManager::getInstance().addProvinceSelection(merged_prov, true);
                                    }

                                    auto* province = &province_project.getProvinceForID(merged_prov);
                                    auto preview_data = province_project.getPreviewData(province);

                                    if(root_id == root_id) {
                                        if(action == SelectionManager::Action::SET) {
                                            getProvincePropertiesPane().setProvince(province, preview_data);
                                        } else {
                                            // This will be true if there are already any selections in the
                                            //  list. In other words, the first selection should always
                                            //  populate the properties pane, but subsequent selections
                                            //  should not.
                                            const auto& selected_labels = SelectionManager::getInstance().getSelectedProvinceLabels();
                                            bool has_selections_already = !selected_labels.empty();

                                            // TODO: preview should show the merged provinces combined
                                            getProvincePropertiesPane().setProvince(province, preview_data, has_selections_already);
                                        }
                                    }

                                    if(!skip_select_in_filetree) {
                                        keys.push_back(
                                            Project::Hierarchy::Key{
                                                Project::Hierarchy::ProjectKeys::MAP,
                                                Project::Hierarchy::ProjectKeys::PROVINCES,
                                                Project::Hierarchy::GroupKeys::PROVINCES,
                                                std::to_string(merged_prov)
                                            }
                                        );
                                    } else if(select_in_tree_override.has_value()) {
                                        WRITE_DEBUG("Override key, select ", 
                                                    std::to_string(*select_in_tree_override),
                                                    " instead.");
                                        keys.push_back(*select_in_tree_override);
                                    }

                                    m_drawing_area->addSelection({preview_data, province->bounding_box, province->id});
                                }

                                selectNode(keys, action);
                            }
                            m_drawing_area->queueDraw();
                        }
                        break;
                    case SelectionManager::Action::REMOVE:
                        {
                            auto& province_project = map_project.getProvinceProject();
                            auto&& merged_provinces = province_project.getMergedProvinces(prov_id);

                            if(merged_provinces.empty()) {
                                WRITE_ERROR("Got 0 elements from getMergedProvinces! Cannot de-select.");
                            } else {
                                WRITE_DEBUG("Deselecting ", merged_provinces.size(), " provinces at once.");

                                auto* ppp_province = getProvincePropertiesPane().getProvince();

                                // List of keys for each merged province
                                std::vector<Project::Hierarchy::Key> keys;

                                // Go over every province in the merged province
                                //   and remove it from the selection
                                for(auto&& merged_prov : merged_provinces) {
                                    // Only remove the province from the properties pane if
                                    //  the province there is the same one we are removing
                                    if(ppp_province != nullptr &&
                                       ppp_province->id == merged_prov)
                                    {
                                        // TODO: We should really be changing to the next one in
                                        //   the selection if we have one selected.
                                        ProvincePreviewDrawingArea::DataPtr null_data; // Do not construct
                                        getProvincePropertiesPane().setProvince(nullptr, null_data);
                                    }

                                    keys.push_back(
                                        Project::Hierarchy::Key{
                                            Project::Hierarchy::ProjectKeys::MAP,
                                            Project::Hierarchy::ProjectKeys::PROVINCES,
                                            Project::Hierarchy::GroupKeys::PROVINCES,
                                            std::to_string(merged_prov)
                                        }
                                    );
                                    m_drawing_area->removeSelection({nullptr, {}, merged_prov});
                                }

                                selectNode(keys, action);
                            }

                            m_drawing_area->queueDraw();
                        }
                        break;
                    case SelectionManager::Action::CLEAR:
                        ProvincePreviewDrawingArea::DataPtr null_data; // Do not construct
                        getProvincePropertiesPane().setProvince(nullptr, null_data);

                        getStatePropertiesPane().setState(nullptr);

                        selectNode({}, SelectionManager::Action::CLEAR);

                        m_drawing_area->setSelection();
                        m_drawing_area->queueDraw();

                        break;
                }
            });

        SelectionManager::getInstance().setOnSelectStateCallback(
            [this](StateID state_id, SelectionManager::Action action)
            {
                auto& history_project = Driver::getInstance().getProject()->get().getHistoryProject();

                switch(action) {
                    case SelectionManager::Action::SET:
                    case SelectionManager::Action::ADD:
                        // TODO: we can't use the above is_already_selected or
                        //   has_selections_already variables since those are
                        //   referring to provinces, so we need to do similar
                        //   calculations again but for states
                        history_project.getStateProject().getStateForID(state_id).andThen([this](auto state_ref)
                        {
                            auto* state = &state_ref.get();
                            getStatePropertiesPane().setState(state);
                        });

                        // TODO: Update state drawing area once we have that
                        //   Use different behavior for add/select
                        break;
                    case SelectionManager::Action::REMOVE:
                        // TODO: Update state drawing area once we have that for removal
                    case SelectionManager::Action::CLEAR:
                        getStatePropertiesPane().setState(nullptr);
                        break;
                }
            });
    }

    // ActionManager callbacks
    {
        Action::ActionManager::getInstance().setOnDoActionCallback([this](const auto&...)
        {
            m_toolbar->updateUndoRedoButtons();

            // Update each properties pane that an action has been performed
            getProvincePropertiesPane().updateProperties(SelectionManager::getInstance().getSelectedProvinceCount() > 1);
            getStatePropertiesPane().updateProperties(SelectionManager::getInstance().getSelectedStateCount() > 1);
        });
        Action::ActionManager::getInstance().setOnUndoActionCallback([this](const auto&...)
        {
            m_toolbar->updateUndoRedoButtons();

            // Update each properties pane that an action has been undone
            getProvincePropertiesPane().updateProperties(SelectionManager::getInstance().getSelectedProvinceCount() > 1);
            getStatePropertiesPane().updateProperties(SelectionManager::getInstance().getSelectedStateCount() > 1);
        });
        Action::ActionManager::getInstance().setOnRedoActionCallback([this](const auto&...)
        {
            m_toolbar->updateUndoRedoButtons();

            // Update each properties pane that an action has been redone
            getProvincePropertiesPane().updateProperties(SelectionManager::getInstance().getSelectedProvinceCount() > 1);
            getStatePropertiesPane().updateProperties(SelectionManager::getInstance().getSelectedStateCount() > 1);
        });

    }

    // File Tree callbacks
    {
        setOnNodeDoubleClickCallback([this](Project::Hierarchy::INodePtr node,
                                            GdkEventType /* type */,
                                            uint32_t /* button */)
        {
            // On double clicking a link node, navigate us to that node in the tree
            if(node->getType() == Project::Hierarchy::Node::Type::LINK) {
                if(auto lnode = std::dynamic_pointer_cast<Project::Hierarchy::ILinkNode>(node);
                        lnode != nullptr)
                {
                    auto key = getKeyForNode(lnode->getLinkedNode());
                    if(IS_FAILURE(key)) {
                        WRITE_ERROR("Failed to get key for node ",
                                std::to_string(*node),
                                ", cannot navigate to it.");
                        return;
                    }

                    WRITE_DEBUG("Got key ", std::to_string(*key),
                                ", selecting node in tree.");
                    selectNode({*key}, SelectionManager::Action::SET);
                } else {
                    WRITE_WARN("Node ", std::to_string(*node, true),
                               " is marked as a link node, but we failed to"
                               " cast it to an ILinkNode object.");
                }
            }

            // Do we want to perform any other actions when double clicking a node?
        });
    }
}

auto HMDT::GUI::MainWindow::getLogViewerWindow()
    -> OptionalReference<LogViewerWindow>
{
    if(m_log_viewer_window == nullptr) {
        return std::nullopt;
    } else {
        return *m_log_viewer_window;
    }
}

void HMDT::GUI::MainWindow::buildToolbar() {
    m_toolbar = addWidget<Toolbar>(*this);

    m_toolbar->init();
}

/**
 * @brief Builds the view pane, which is where the map gets rendered to.
 */
void HMDT::GUI::MainWindow::buildViewPane() {
    m_left_pane->pack2(*std::get<Gtk::Frame*>(setActiveChild(new Gtk::Frame)), true, false);

    buildDrawingArea();
}

/**
 * @brief Updates a part of the main window.
 *
 * @param part Which part to update
 * @param data User data that the specific part updater needs
 */
void HMDT::GUI::MainWindow::updatePart(const PartType& part, const std::any& data) noexcept
{
    switch(part) {
        case PartType::MAIN:
        case PartType::DRAWING_AREA:
        case PartType::PROPERTIES_PANE:
            WRITE_WARN("Asked to update part #", (int)part, ", but that hasn't "
                       "been implemented yet.");
            break;
        case PartType::FILE_TREE:
            updateFileTree(std::any_cast<Project::Hierarchy::Key>(data));
    }
}

auto HMDT::GUI::MainWindow::getPart(const PartType& part) noexcept
    -> BaseMainWindow&
{
    switch(part) {
        case PartType::MAIN:
            return *this;
        case PartType::DRAWING_AREA:
            return *thisAs<MainWindowDrawingAreaPart>();
        case PartType::PROPERTIES_PANE:
            return *thisAs<MainWindowPropertiesPanePart>();
        case PartType::FILE_TREE:
            return *thisAs<MainWindowFileTreePart>();
    }

    UNREACHABLE();
}

auto HMDT::GUI::MainWindow::getPart(const PartType& part) const noexcept
    -> const BaseMainWindow&
{
    switch(part) {
        case PartType::MAIN:
            return *this;
        case PartType::DRAWING_AREA:
            return *thisAs<MainWindowDrawingAreaPart>();
        case PartType::PROPERTIES_PANE:
            return *thisAs<MainWindowPropertiesPanePart>();
        case PartType::FILE_TREE:
            return *thisAs<MainWindowFileTreePart>();
    }

    UNREACHABLE();
}

Gtk::Orientation HMDT::GUI::MainWindow::getDisplayOrientation() const {
    return Gtk::Orientation::ORIENTATION_VERTICAL;
}

/**
 * @brief Adds the given widget to the currently active widget
 *
 * @param widget The widget to add
 */
void HMDT::GUI::MainWindow::addWidgetToParent(Gtk::Widget& widget) {
    std::visit(overloaded {
        [&widget](auto&& child) {
            child->add(widget);
        },
        [this, &widget](std::monostate) {
            Window::addWidgetToParent(widget);
        }
    }, getActiveChild());
}

/**
 * @brief Creates a dialog box and, if successful, will create and set a new
 *        project on the Driver
 */
void HMDT::GUI::MainWindow::newProject() {
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
            if(IS_FAILURE(project->save(false))) {
                Gtk::MessageDialog dialog(*this, gettext("Failed to save project."), false,
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

void HMDT::GUI::MainWindow::openProject() {
    Driver::UniqueProject project(new Driver::HProject);

    // Allocate this on the stack so that it gets automatically cleaned up
    //  when we finish
    std::string path;

    NativeDialog::FileDialog dialog(gettext("Choose a project file."),
                                    NativeDialog::FileDialog::SELECT_FILE);
    // dialog.setDefaultPath() // TODO: Start in the installation directory/Documents
    dialog.addFilter(gettext("Project Files"), "hoi4proj")
          .addFilter(gettext("All files"), "*")
          .setAllowsMultipleSelection(false)
          .setDecideHandler([&path](const NativeDialog::Dialog& dialog) {
                auto& fdlg = dynamic_cast<const NativeDialog::FileDialog&>(dialog);
                path = fdlg.selectedPathes().front();
          }).show();

    if(!path.empty()) {
        project->setPath(path);

        MaybeVoid result;
        try {
            result = project->load();
        } catch(const std::exception& exc) {
            WRITE_ERROR("Caught unhandled exception during project load! what()=",
                        exc.what());
            result = STATUS_UNEXPECTED;
        }

        if(IS_FAILURE(result)) {
            Gtk::MessageDialog err_diag(gettext("Failed to open file."), false,
                                        Gtk::MESSAGE_ERROR);
            err_diag.run();
            return;
        }
    }

    if(project->getToolVersion() != TOOL_VERSION) {
        std::stringstream message_ss;
        message_ss << gettext("This project was built with a different tool version: '")
                   << project->getToolVersion() << "' != '" << TOOL_VERSION
                   << "\'\n"
                   << gettext("Color data may be generated differently.");

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
void HMDT::GUI::MainWindow::onProjectOpened() {
    // Enable all actions that can only be done on an opened project
    getAction("save")->set_enabled(true);
    getAction("close")->set_enabled(true);
    getAction("recalc_coasts")->set_enabled(true);
    getAction("export_project")->set_enabled(true);
    getAction("export_project_as")->set_enabled(true);
    getAction("generate_template_rivers")->set_enabled(true);
    getAction("add_item")->set_enabled(true);

    // Issue callback to the properties pane to inform it that a project has
    //   been opened
    MainWindowPropertiesPanePart::onProjectOpened();
    auto result = MainWindowFileTreePart::onProjectOpened();
    if(IS_FAILURE(result)) {
        Gtk::MessageDialog dialog(*this, gettext("Failed to signal project opening to file tree."), false,
                                  Gtk::MESSAGE_ERROR);
        dialog.run();
        return;
    }
}

/**
 * @brief Called when a project is closed
 */
void HMDT::GUI::MainWindow::onProjectClosed() {
    // Have the drawing area forget the data it was set to render
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        opt_project->get().getMapProject().getMapData()->close();
    }
    m_drawing_area->queueDraw();

    // Disable all actions that can only be done on an opened project
    getAction("save")->set_enabled(false);
    getAction("close")->set_enabled(false);
    getAction("recalc_coasts")->set_enabled(false);
    getAction("export_project")->set_enabled(false);
    getAction("export_project_as")->set_enabled(false);
    getAction("generate_template_rivers")->set_enabled(false);
    getAction("add_item")->set_enabled(false);

    {
        ProvincePreviewDrawingArea::DataPtr null_data; // Do not construct
        getProvincePropertiesPane().setProvince(nullptr, null_data);

        m_drawing_area->setSelection();
        m_drawing_area->queueDraw();
    }
}

/**
 * @brief Saves the currently set Driver project (if one is in fact set)
 */
void HMDT::GUI::MainWindow::saveProject() {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        // Make sure the user is notified if we failed to save the project
        if(IS_FAILURE(project.save())) {
            Gtk::MessageDialog dialog(*this, gettext("Failed to save file."), false,
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
void HMDT::GUI::MainWindow::saveProjectAs(const std::string& dtitle) {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        // Allocate this on the stack so that it gets automatically cleaned up
        //  when we finish
        NativeDialog::FileDialog dialog(dtitle,
                                        NativeDialog::FileDialog::SELECT_FILE |
                                        NativeDialog::FileDialog::SELECT_TO_SAVE);
        // dialog.setDefaultPath() // TODO: Start in the installation directory/Documents
        dialog.addFilter(gettext("Project Files"), "hoi4proj")
              .addFilter(gettext("All files"), "*")
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

void HMDT::GUI::MainWindow::exportProject() {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        opt_project->get().setPromptCallback(
            [this](const std::string& message,
                      const std::vector<std::string>& opts,
                      const Project::IProject::PromptType& type)
                -> uint32_t
            {
                Gtk::MessageType gtk_msg_type = Gtk::MESSAGE_INFO;
                switch(type) {
                    case Project::IProject::PromptType::INFO:
                        gtk_msg_type = Gtk::MESSAGE_INFO;
                        break;
                    case Project::IProject::PromptType::WARN:
                        gtk_msg_type = Gtk::MESSAGE_WARNING;
                        break;
                    case Project::IProject::PromptType::ERROR:
                    case Project::IProject::PromptType::ALERT:
                        gtk_msg_type = Gtk::MESSAGE_ERROR;
                        break;
                    case Project::IProject::PromptType::QUESTION:
                        gtk_msg_type = Gtk::MESSAGE_QUESTION;
                        break;
                }

                // Create a dialog with no buttons, as we will just add the ones
                //   specified in opts.
                Gtk::MessageDialog dialog(*this,
                                          message,
                                          true /* use_markup */,
                                          gtk_msg_type /* type */,
                                          Gtk::BUTTONS_NONE /* buttons */);
                for(uint32_t i = 0; i < opts.size(); ++i) {
                    dialog.add_button(opts.at(i), i);
                }

                return dialog.run();
            });

        if(auto res = opt_project->get().export_(); IS_FAILURE(res)) {
            std::stringstream ss;
            ss << gettext("Reason: ") << "0x"
               << std::hex << res.error().value() << std::dec
               << " '" << res.error().message() << "'";

            Gtk::MessageDialog dialog(*this, gettext("Failed to export project."),
                                      false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text(ss.str());
            dialog.run();
        } else {
            std::stringstream ss;
            ss << "<b>" << gettext("Successfully exported project.") << "</b>";
            Gtk::MessageDialog dialog(*this, ss.str(), true, Gtk::MESSAGE_INFO);
            dialog.run();
        }

        // Make sure we reset before checking for errors
        opt_project->get().resetPromptCallback();
    }
}

/**
 * @brief Performs a exportAs operation, asking the user for a new file location
 *        to save to before calling exportProject()
 */
void HMDT::GUI::MainWindow::exportProjectAs(const std::string& dtitle) {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& project = opt_project->get();

        auto default_export_root = project.getDefaultExportRoot();

        // Allocate this on the stack so that it gets automatically cleaned up
        //  when we finish
        NativeDialog::FileDialog dialog(dtitle,
                                        NativeDialog::FileDialog::SELECT_DIR);
        dialog.setDefaultPath(default_export_root.generic_string());
        dialog.setAllowsMultipleSelection(false)
              .setDecideHandler([&project](const NativeDialog::Dialog& dialog) {
                    auto& fdlg = dynamic_cast<const NativeDialog::FileDialog&>(dialog);
                    auto&& paths = fdlg.selectedPathes();

                    project.setExportRoot(paths.front());
              }).show();

        // Call this outside of the DecideHandler because we want to ensure that
        //   the dialog window is closed first
        exportProject();
    }
}

