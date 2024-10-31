
#include "ProvincePropertiesPane.h"

#include <libintl.h>
#include <numeric>

#include "gtkmm/messagedialog.h"
#include "gtkmm/label.h"
#include "gtkmm/frame.h"

#include "Constants.h"
#include "Logger.h"
#include "Util.h"
#include "Options.h"

#include "ActionManager.h"
#include "SetPropertyAction.h"
#include "CreateRemoveContinentAction.h"
#include "CreateRemoveStateAction.h"

#include "Driver.h"
#include "SelectionManager.h"
#include "MainWindowFileTreePart.h"

#include "NodeKeyNames.h"

HMDT::GUI::ProvincePropertiesPane::ProvincePropertiesPane(BaseMainWindow& main_window):
    m_main_window(main_window),
    m_province(nullptr),
    m_box(Gtk::ORIENTATION_VERTICAL),
    m_is_updating_properties(false),
    m_value_changed_callback([](auto&&...){})
{ }

Gtk::ScrolledWindow& HMDT::GUI::ProvincePropertiesPane::getParent() {
    return m_parent;
}

/**
 * @brief Initializes every component of the pane
 */
void HMDT::GUI::ProvincePropertiesPane::init() {
    // Note: The empty labels are here for spacing purposes so that the fields
    //   aren't too bunched up

    m_parent.set_size_request(MINIMUM_PROPERTIES_PANE_WIDTH, -1);
    m_parent.add(m_box);
    addWidget<Gtk::Label>("");

    auto* preview_frame = addWidget<Gtk::Frame>();
    preview_frame->add(m_preview_area);
    addWidget<Gtk::Label>("");

    buildIsCoastalField();
    addWidget<Gtk::Label>("");

    buildProvinceTypeField();
    addWidget<Gtk::Label>("");

    buildTerrainTypeField();
    addWidget<Gtk::Label>("");

    buildContinentField();
    addWidget<Gtk::Label>("");

    buildStateCreationButton();
    addWidget<Gtk::Label>("");

    buildMergeProvincesButton();
    addWidget<Gtk::Label>("");

    buildMergedListWindow();
    addWidget<Gtk::Label>("");

    setEnabled(false);

    m_parent.show_all();
}

void HMDT::GUI::ProvincePropertiesPane::buildIsCoastalField() {
    m_is_coastal_button = addWidget<Gtk::CheckButton>(gettext("Is Coastal"));

    m_is_coastal_button->signal_toggled().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_province != nullptr) {
            Action::ActionManager::getInstance().doAction(
                &NewSetPropertyAction(m_province, coastal,
                                     m_is_coastal_button->get_active())
                    ->onValueChanged([this](const auto& old, const auto& _new)
                    {
                        WRITE_DEBUG("Update coastal from ", old, " to ", _new);

                        m_value_changed_callback(
                            Project::Hierarchy::Key{
                                Project::Hierarchy::ProjectKeys::MAP,
                                Project::Hierarchy::ProjectKeys::PROVINCES,
                                Project::Hierarchy::GroupKeys::PROVINCES,
                                std::to_string(m_province->id),
                                Project::Hierarchy::ProvinceKeys::COASTAL
                            });
                    })
            );
        }
    });
}

void HMDT::GUI::ProvincePropertiesPane::buildProvinceTypeField() {
    addWidget<Gtk::Label>(gettext("Province Type"));

    m_provtype_menu = addWidget<Gtk::ComboBoxText>();

    // You can only choose one of these 3 province types
    m_provtype_menu->append(gettext("Land"));
    m_provtype_menu->append(gettext("Sea"));
    m_provtype_menu->append(gettext("Lake"));

    m_provtype_menu->set_active(0);
    m_provtype_menu->signal_changed().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_province != nullptr) {
            // Only set the province type if it is set to a valid one
            if(auto current = m_provtype_menu->get_active_row_number();
                    current != -1)
            {
                Action::ActionManager::getInstance().doAction(
                    &NewSetPropertyAction(m_province, type,
                                         static_cast<ProvinceType>(current + 1))
                    ->onValueChanged([this](const auto& old, const auto& _new)
                    {
                        WRITE_DEBUG("Update type from ", old, " to ", _new);

                        m_value_changed_callback(
                            Project::Hierarchy::Key{
                                Project::Hierarchy::ProjectKeys::MAP,
                                Project::Hierarchy::ProjectKeys::PROVINCES,
                                Project::Hierarchy::GroupKeys::PROVINCES,
                                std::to_string(m_province->id),
                                Project::Hierarchy::ProvinceKeys::TYPE
                            });
                    })
                );
            } else {
                WRITE_ERROR("Province type somehow set to an invalid index: ",
                           m_provtype_menu->get_active_row_number());
            }
        }
    });
}

void HMDT::GUI::ProvincePropertiesPane::buildTerrainTypeField() {
    addWidget<Gtk::Label>(gettext("Terrain Type"));
    m_terrain_menu = addWidget<Gtk::ComboBoxText>();

    // TODO: Add options, how do we know which terrain types are valid?
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& map_project = opt_project->get().getMapProject();
        const auto& terrains = map_project.getTerrains();

        for(auto&& terrain : terrains) {
            m_terrain_menu->append(terrain.getIdentifier());
        }
    }

    m_terrain_menu->set_active(0);
    m_terrain_menu->signal_changed().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_province != nullptr) {
            // TODO: Verify that the active text is a valid terrain type
            Action::ActionManager::getInstance().doAction(
                &NewSetPropertyAction(m_province, terrain,
                                     m_terrain_menu->get_active_text())
                    ->onValueChanged([this](const auto& old, const auto& _new)
                    {
                        WRITE_DEBUG("Update terrain from ", old, " to ", _new);

                        m_value_changed_callback(
                            Project::Hierarchy::Key{
                                Project::Hierarchy::ProjectKeys::MAP,
                                Project::Hierarchy::ProjectKeys::PROVINCES,
                                Project::Hierarchy::GroupKeys::PROVINCES,
                                std::to_string(m_province->id),
                                Project::Hierarchy::ProvinceKeys::TERRAIN
                            });
                    })
                );
        }
    });
}

void HMDT::GUI::ProvincePropertiesPane::buildContinentField() {
    addWidget<Gtk::Label>(gettext("Continent"));

    m_continent_menu = addWidget<Gtk::ComboBoxText>();
    m_continent_menu->append("None");

    m_continent_menu->signal_changed().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_province != nullptr) {
            Action::ActionManager::getInstance().doAction(
                &NewSetPropertyAction(m_province, continent,
                                     m_continent_menu->get_active_text())
                    ->onValueChanged([this](const auto& old, const auto& _new)
                    {
                        WRITE_DEBUG("Update continent from ", old, " to ", _new);

                        m_value_changed_callback(
                            Project::Hierarchy::Key{
                                Project::Hierarchy::ProjectKeys::MAP,
                                Project::Hierarchy::ProjectKeys::PROVINCES,
                                Project::Hierarchy::GroupKeys::PROVINCES,
                                std::to_string(m_province->id),
                                Project::Hierarchy::ProvinceKeys::CONTINENT
                            });
                    })
                );
        }
    });

    {
        // Add+Remove buttons for continents
        Gtk::Box* add_rem_box = addWidget<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);

        m_add_button = manage(new Gtk::Button("+"));
        m_rem_button = manage(new Gtk::Button("-"));

        add_rem_box->add(*m_add_button);
        add_rem_box->add(*m_rem_button);

        // Default both of these to be insensitive, we'll decide later if they
        //   should be made sensitive
        m_add_button->set_sensitive(false);
        m_rem_button->set_sensitive(false);

        m_add_button->signal_clicked().connect([this]() {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project)
            {
                auto& map_project = opt_project->get().getMapProject();
                const auto& continents = map_project.getContinentProject().getContinentList();

                Gtk::Dialog add_dialog(gettext("Add a continent"));
                Gtk::Entry continent_name_entry;
                Gtk::Label entry_label(gettext("Name of the new continent:"));

                Gtk::Bin* bin = reinterpret_cast<Gtk::Bin*>(add_dialog.get_child());

                bin->add(entry_label);
                bin->add(continent_name_entry);

                auto confirm_button = add_dialog.add_button(gettext("Confirm"), Gtk::RESPONSE_ACCEPT);
                confirm_button->set_sensitive(false);

                add_dialog.add_button(gettext("Cancel"), Gtk::RESPONSE_CANCEL);

                // Set up a signal so that we can update if the confirm button
                //  should be activated
                continent_name_entry.signal_changed().connect([&confirm_button,
                                                               &continent_name_entry,
                                                               &continents]()
                {
                    // The button is sensitive IIF the entered text is a valid
                    //  continent
                    auto text = continent_name_entry.get_text();
                    confirm_button->set_sensitive(text != "None" &&
                                                  !text.empty() &&
                                                  continents.count(text) == 0);
                });

                add_dialog.show_all_children();

                const int result = add_dialog.run();
                switch(result) {
                    case Gtk::RESPONSE_ACCEPT:
                        if(Action::ActionManager::getInstance().doAction(
                            new Action::CreateRemoveContinentAction(map_project,
                                                                    continent_name_entry.get_text(),
                                                                    Action::CreateRemoveContinentAction::Type::CREATE)))
                        {
                            // Rebuild the continent menu here so that they remain
                            //  in the same order as in the internal std::set
                            rebuildContinentMenu(continents);
                        }
                        break;
                    case Gtk::RESPONSE_CANCEL:
                    default:
                        return;
                }

                // Make the remove button active again
                m_rem_button->set_sensitive(true);
            }
        });

        m_rem_button->signal_clicked().connect([this]() {
            if(auto opt_project = Driver::getInstance().getProject(); opt_project)
            {
                auto& map_project = opt_project->get().getMapProject();
                const auto& continents = map_project.getContinentProject().getContinentList();

                Gtk::Dialog rem_dialog(gettext("Remove a Continent"));
                Gtk::Entry continent_name_entry;
                Gtk::Bin* bin = reinterpret_cast<Gtk::Bin*>(rem_dialog.get_child());

                // Add the Confirm and Cancel buttons to the dialog
                bin->add(continent_name_entry);
                auto confirm_button = rem_dialog.add_button(gettext("Confirm"), Gtk::RESPONSE_ACCEPT);
                confirm_button->set_sensitive(false);

                rem_dialog.add_button(gettext("Cancel"), Gtk::RESPONSE_CANCEL);

                // Set up a signal so that we can update if the confirm button
                //  should be activated
                continent_name_entry.signal_changed().connect([&confirm_button,
                                                               &continent_name_entry,
                                                               &continents]()
                {
                    // The button is sensitive IIF the entered text is a valid
                    //  continent
                    auto text = continent_name_entry.get_text();
                    confirm_button->set_sensitive(text != "None" &&
                                                  !text.empty() &&
                                                  continents.count(text) == 0);
                });

                rem_dialog.show_all_children();

                switch(rem_dialog.run()) {
                    case Gtk::RESPONSE_ACCEPT:
                        if(Action::ActionManager::getInstance().doAction(
                            new Action::CreateRemoveContinentAction(map_project,
                                                                    continent_name_entry.get_text(),
                                                                    Action::CreateRemoveContinentAction::Type::REMOVE)))
                        {
                            // Rebuild the continent menu here so that they remain
                            //  in the same order as in the internal std::set
                            rebuildContinentMenu(continents);
                        }
                        break;
                    case Gtk::RESPONSE_CANCEL:
                    default:
                        return;
                }

                // Make the remove button active again
                m_rem_button->set_sensitive(!continents.empty());
            }
        });
    }

    // Simulate a project opening to initialize the contents of the continent
    //  menu if a project somehow is already opened when we are building the
    //  properties menu
    onProjectOpened();
}

void HMDT::GUI::ProvincePropertiesPane::buildStateCreationButton() {
    m_create_state_button = addWidget<Gtk::Button>(gettext("Create State"));

    m_create_state_button->signal_clicked().connect([this]() {
        if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
            auto& history_project = opt_project->get().getHistoryProject();
            auto& state_project = history_project.getStateProject();

            auto selected = SelectionManager::getInstance().getSelectedProvinceLabels();

            Project::Hierarchy::Key key{
                Project::Hierarchy::ProjectKeys::HISTORY,
                Project::Hierarchy::ProjectKeys::STATES,
                Project::Hierarchy::GroupKeys::STATES
            };

            Action::ActionManager::getInstance().doAction(
                &(new Action::CreateRemoveStateAction(history_project,
                                                      std::vector<ProvinceID>(selected.begin(),
                                                                              selected.end())
                    )
                  )
                  ->onValueChanged([this, &state_project, key](const StateID& id)
                  {
                      m_value_changed_callback(key);

                      // Only select the state if it exists/is valid
                      if(state_project.isValidStateID(id)) {
                          SelectionManager::getInstance().selectState(id);
                      }

                      // TODO: If we have a State view, we should switch to it here
                      // TODO: We should also switch from the province properties pane to
                      //       the state properties pane
                      return true;
                  })
                  .onCreate([this, &state_project, key](const StateID& id) {
                      // This will run right before onValueChanged, so make
                      //   sure to update the hierarchy now
                      auto& mwft = m_main_window.getPartAs<MainWindowFileTreePart>(BaseMainWindow::PartType::FILE_TREE);

                      auto maybe_state = state_project.getStateForID(id);
                      RETURN_VALUE_IF_ERROR(maybe_state, false);

                      auto state_node = state_project.createStateNode(id, *maybe_state);

                      auto result = mwft.addNodeToHierarchy(key, state_node);
                      RETURN_VALUE_IF_ERROR(result, false);

                      // No need to update the tree here, as that will happen
                      //   in onValueChanged
                      return true;
                  })
                  .onRemove([this, key](const State& state) {
                      // This will run right before onValueChanged, so make
                      //   sure to update the hierarchy now
                      auto& mwft = m_main_window.getPartAs<MainWindowFileTreePart>(BaseMainWindow::PartType::FILE_TREE);

                      // TODO: Using the state name is really bad, we should
                      //   honestly instead be using StateID, but we won't be
                      //   able to do that until States have been migrated over
                      //   to using UUID instead of an int for their ID
                      auto result = mwft.removeNodeFromHierarchy(key / state.name);
                      RETURN_VALUE_IF_ERROR(result, false);

                      // No need to update the tree here, as that will happen
                      //   in onValueChanged

                      return true;
                  })
            );
        }
    });
}


/**
 * @brief Creates the button to merge two or more provinces together
 */
void HMDT::GUI::ProvincePropertiesPane::buildMergeProvincesButton() {
    m_merge_provinces_button = addWidget<Gtk::Button>(gettext("Merge Provinces"));

    m_merge_provinces_button->signal_clicked().connect([]() {
        if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
            auto& province_project = opt_project->get().getMapProject().getProvinceProject();

            auto selected = SelectionManager::getInstance().getSelectedProvinceLabels();

            // Get a root province project 
            auto it = selected.begin();
            auto root_id = province_project.getRootProvinceParent(*it);
            if(IS_FAILURE(root_id)) {
                WRITE_ERROR("Failed to get root province parent of id ", *it);
                return;
            }

            for(; it != selected.end(); ++it) {
                // Merge all selected provinces together (but take care not to
                //   merge the root into itself
                if(*it != root_id->get().id) {
                    province_project.mergeProvinces(root_id->get().id, *it);
                }
            }

            // Re-select the merged province now to update the pane.
            //   We only need to do this on one of them, and it will trigger the
            //   code to select all merged provinces
            SelectionManager::getInstance().selectProvince(root_id->get().id);
        }
    });

    // Default this to being disabled
    m_merge_provinces_button->set_sensitive(false);
}

/**
 * @brief Creates the list of merged provinces
 */
void HMDT::GUI::ProvincePropertiesPane::buildMergedListWindow() {
    auto* frame = addWidget<Gtk::Frame>();

    m_merged_list_window = new ProvinceListWindow(
        [](auto&& id) {
            SelectionManager::getInstance().selectProvince(id);
        } /* callback */,
        ProvinceListWindow::ProvinceRowInfo {
            std::string("   ID ") /* label_prefix */,
            std::string("×") /* remove_button_label */,
            true /* is_destructive */,
            true /* remove_self */,
            [](const ProvinceID& id) -> bool {
                if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
                    auto& map_project = opt_project->get().getMapProject();

                    // Only bother actually removing the province from the state if the
                    //  ID is valid (if it is invalid, then how did we even add it?)
                    if(auto& prov_project = map_project.getProvinceProject();
                            prov_project.isValidProvinceID(id))
                    {
                        auto& province = prov_project.getProvinceForID(id);

                        // Deselect the province
                        SelectionManager::getInstance().removeProvinceSelection(id);

                        // If there is only a single province selected, then we want to
                        //  also deselect the state
                        if(auto selected = SelectionManager::getInstance().getSelectedProvinces();
                                selected.size() == 1)
                        {
                            SelectionManager::getInstance().removeStateSelection(province.state);
                        }

                        // Unmerge the state last, after all deselections
                        if(auto result = prov_project.unmergeProvince(id);
                           IS_FAILURE(result))
                        {
                            WRITE_ERROR("Failed with error to unmerge province ",
                                        id, " from its parent.");
                        }
                    } else {
                        WRITE_WARN("Invalid province ID ", id, ". Cannot unmerge.");
                    }

                    return true;
                }

                return false;
            } /* remove_button_callback */
        }
    );

    frame->add(*m_merged_list_window);

    m_merged_list_window->show_all();
}

/**
 * @brief Updates the list of merged provinces for a new province
 *
 * @param prov The province to update with
 */
void HMDT::GUI::ProvincePropertiesPane::updateMergedListElements(const Province* prov)
{
    // Next only add the new provinces to the list if there are provinces to add
    if(prov != nullptr) {
        if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
            auto& province_project = opt_project->get().getMapProject().getProvinceProject();

            auto&& merged_provinces = province_project.getMergedProvinces(prov->id);

            WRITE_DEBUG("Populating list with ", merged_provinces.size(), " provinces.");

            m_merged_list_window->setListElements(merged_provinces);
        }
    } else {
        // Clear out the list if prov is null
        WRITE_DEBUG("Populating list with 0 provinces.");
        m_merged_list_window->setListElements({});
    }
}

void HMDT::GUI::ProvincePropertiesPane::addWidgetToParent(Gtk::Widget& widget) {
    m_box.add(widget);
}

/**
 * @brief Sets the sensitivity/enabled state of every field
 *
 * @param enabled
 */
void HMDT::GUI::ProvincePropertiesPane::setEnabled(bool enabled) {
    m_is_coastal_button->set_sensitive(enabled);
    m_provtype_menu->set_sensitive(enabled);
    m_terrain_menu->set_sensitive(enabled);
    m_continent_menu->set_sensitive(enabled);

    m_create_state_button->set_sensitive(enabled);
}

void HMDT::GUI::ProvincePropertiesPane::setProvince(Province* prov,
                                                    ProvincePreviewDrawingArea::DataPtr preview_data,
                                                    bool is_multiselect)
{
    m_province = prov;

    setPreview(preview_data);

    setEnabled(m_province != nullptr && !is_multiselect);

    // Set this to be enabled afterwards without worrying about multiselect
    if(is_multiselect) {
        m_create_state_button->set_sensitive(m_province != nullptr);
    }

    updateProperties(prov, is_multiselect);
}

auto HMDT::GUI::ProvincePropertiesPane::getProvince() -> Province* {
    return m_province;
}

void HMDT::GUI::ProvincePropertiesPane::setPreview(ProvincePreviewDrawingArea::DataPtr preview_data)
{
    if(m_province == nullptr) {
        m_preview_area.setData(preview_data, 0, 0);
    } else {
        auto&& [width, height] = calcDims(m_province->bounding_box);
        m_preview_area.setData(preview_data, width, height);
    }
}

void HMDT::GUI::ProvincePropertiesPane::onResize() {
    m_preview_area.calcScale();
}

void HMDT::GUI::ProvincePropertiesPane::updateProperties(bool is_multiselect)
{
    updateProperties(m_province, is_multiselect);
}

/**
 * @brief Will update all of the values stored in every field to the given
 *        province. If prov is nullptr, then the values are changed to defaults.
 *
 * @param prov The province to update the properties to.
 */
void HMDT::GUI::ProvincePropertiesPane::updateProperties(const Province* prov,
                                                         bool is_multiselect)
{
    m_is_updating_properties = true;

    if(prov == nullptr || is_multiselect) {
        // Set every field to some sort of sane default
        m_is_coastal_button->set_active(false);
        m_provtype_menu->set_active(0);
        m_terrain_menu->set_active_text("unknown");
        m_continent_menu->set_active_text("None");
    } else {
        // Set every field to prov
        m_is_coastal_button->set_active(prov->coastal);
        m_provtype_menu->set_active(static_cast<int>(prov->type) - 1);
        m_terrain_menu->set_active_text(prov->terrain.empty() ? "unknown" : prov->terrain.c_str());
        m_continent_menu->set_active_text(prov->continent.empty() ? "None" : prov->continent.c_str());
    }

    // Only allow merging provinces if at least two are selected
    m_merge_provinces_button->set_sensitive(prov != nullptr && is_multiselect);

    updateMergedListElements(prov);

    m_is_updating_properties = false;
}

/**
 * @brief Rebuilds the continents menu
 *
 * @param continents The continents to be in the new menu
 */
void HMDT::GUI::ProvincePropertiesPane::rebuildContinentMenu(const std::set<std::string>& continents)
{
    m_continent_menu->remove_all();
    m_continent_menu->append("None");
    for(auto&& c : continents) {
        m_continent_menu->append(c);
    }
}

void HMDT::GUI::ProvincePropertiesPane::onProjectOpened() {
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& map_project = opt_project->get().getMapProject();
        const auto& continents = map_project.getContinentProject().getContinentList();

        rebuildContinentMenu(continents);

        // The remove button only does stuff if there are continents _to_
        //  remove
        m_rem_button->set_sensitive(!continents.empty());
        m_add_button->set_sensitive(true);
    }
}

void HMDT::GUI::ProvincePropertiesPane::setCallbackOnValueChanged(const ValueChangedCallback& callback) noexcept
{
    m_value_changed_callback = callback;
}

