
#include "StatePropertiesPane.h"

#include "gtkmm/messagedialog.h"
#include "gtkmm/label.h"
#include "gtkmm/frame.h"

#include "Constants.h"
#include "Logger.h"
#include "Util.h"

#include "ActionManager.h"
#include "SetPropertyAction.h"

#include "Driver.h"
#include "StyleClasses.h"
#include "SelectionManager.h"

HMDT::GUI::StatePropertiesPane::ProvinceRow::ProvinceRow(Gtk::ListBox* owning_box,
                                                         ProvinceID id):
    m_province_id(id),
    m_owning_box(owning_box),
    m_hbox(),
    m_label(std::string("   ID ") + std::to_string(id)), // Some spaces for left-padding
    m_remove_button("×")
{
    m_remove_button.set_relief(Gtk::RELIEF_NONE);
    m_remove_button.get_style_context()->add_class(StyleClasses::DESTRUCTIVE_ACTION.data());
    m_remove_button.signal_clicked().connect([this]() {
        if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
            auto& map_project = opt_project->get().getMapProject();

            // Only bother actually removing the province from the state if the
            //  ID is valid (if it is invalid, then how did we even add it?)
            if(map_project.getProvinceProject().isValidProvinceLabel(m_province_id))
            {
                auto& province = map_project.getProvinceProject().getProvinceForLabel(m_province_id);

                // Deselect the province
                SelectionManager::getInstance().removeProvinceSelection(m_province_id);

                // If there is only a single province selected, then we want to
                //  also deselect the state
                if(auto selected = SelectionManager::getInstance().getSelectedProvinces();
                        selected.size() == 1)
                {
                    SelectionManager::getInstance().removeStateSelection(province.state);
                }

                // Remove from the state as well, do this after all deselections
                map_project.removeProvinceFromState(province);
            } else {
                WRITE_WARN("Invalid province ID ", m_province_id, ". Cannot remove from the state.");
            }

            // Either way though, remove it from the list
            m_owning_box->remove(*this);
        }
    });

    m_hbox.pack_start(m_label, Gtk::PACK_SHRINK);
    m_hbox.pack_start(m_remove_button, Gtk::PACK_EXPAND_PADDING);
    add(m_hbox);

    show_all_children();
}

auto HMDT::GUI::StatePropertiesPane::ProvinceRow::getProvinceID() const
    -> ProvinceID
{
    return m_province_id;
}

HMDT::GUI::StatePropertiesPane::StatePropertiesPane():
    m_state(nullptr),
    m_box(Gtk::ORIENTATION_VERTICAL)
{ }

Gtk::ScrolledWindow& HMDT::GUI::StatePropertiesPane::getParent() {
    return m_parent;
}

/**
 * @brief Initializes every component of the pane
 */
void HMDT::GUI::StatePropertiesPane::init() {
    // Note: The empty labels are here for spacing purposes so that the fields
    //   aren't too bunched up

    m_parent.set_size_request(MINIMUM_PROPERTIES_PANE_WIDTH, -1);
    m_parent.add(m_box);
    addWidget<Gtk::Label>("");

    addWidget<Gtk::Label>("State Name: ");
    buildNameField();
    addWidget<Gtk::Label>("");

    addWidget<Gtk::Label>("Manpower: ");
    buildManpowerField();
    addWidget<Gtk::Label>("");

    addWidget<Gtk::Label>("Category: ");
    buildCategoryField();
    addWidget<Gtk::Label>("");

    addWidget<Gtk::Label>("Buildings Max Level Factor: ");
    buildBuildingsMaxLevelFactorField();
    addWidget<Gtk::Label>("");

    buildIsImpassableField();
    addWidget<Gtk::Label>("");

    addWidget<Gtk::Label>("Provinces: ");
    buildProvinceListField();
    addWidget<Gtk::Label>("");

    buildSelectAllProvincesButton();
    addWidget<Gtk::Label>("");

    buildDeleteStateButton();
    addWidget<Gtk::Label>("");

    setEnabled(false);

    m_parent.show_all();
}

void HMDT::GUI::StatePropertiesPane::buildNameField() {
    m_name_field = addWidget<Gtk::Entry>();
    m_name_field->set_placeholder_text("State name...");

    m_name_field->signal_activate().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_state != nullptr && m_state->name != m_name_field->get_text()) {
            Action::ActionManager::getInstance().doAction(
                NewSetPropertyAction(m_state, name,
                                     m_name_field->get_text()));
        }
    });
    m_name_field->signal_focus_out_event().connect([this](GdkEventFocus*) {
        m_name_field->activate();
        return true;
    });
}

void HMDT::GUI::StatePropertiesPane::buildManpowerField() {
    m_manpower_field = addWidget<ConstrainedEntry>();

    // We aren't allowing '-' because manpower cannot be negative
    m_manpower_field->setAllowedChars("0123456789");
    m_manpower_field->set_placeholder_text("Manpower amount (default:0)");

    m_manpower_field->signal_activate().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_state != nullptr &&
           m_state->manpower != std::atoi(m_manpower_field->get_text().c_str()))
        {
            Action::ActionManager::getInstance().doAction(
                NewSetPropertyAction(m_state, manpower,
                                     std::atoi(m_manpower_field->get_text().c_str())));
        }
    });
    m_manpower_field->signal_focus_out_event().connect([this](GdkEventFocus*) {
        m_manpower_field->activate();
        return true;
    });
}

void HMDT::GUI::StatePropertiesPane::buildCategoryField() {
    m_category_field = addWidget<Gtk::Entry>();
    // TODO
    m_category_field->set_sensitive(false);
}

void HMDT::GUI::StatePropertiesPane::buildBuildingsMaxLevelFactorField() {
    m_buildings_max_level_factor_field = addWidget<ConstrainedEntry>();

    m_buildings_max_level_factor_field->setAllowedChars("0123456789.");
    m_buildings_max_level_factor_field->set_placeholder_text("Buildings Max Level Factor (default:1.0)");

    m_buildings_max_level_factor_field->signal_activate().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_state != nullptr &&
           m_state->buildings_max_level_factor != std::atof(m_buildings_max_level_factor_field->get_text().c_str()))
        {
            Action::ActionManager::getInstance().doAction(
                NewSetPropertyAction(m_state, buildings_max_level_factor,
                                     std::atof(m_buildings_max_level_factor_field->get_text().c_str())));
        }
    });
    m_buildings_max_level_factor_field->signal_focus_out_event().connect([this](GdkEventFocus*)
    {
        m_buildings_max_level_factor_field->activate();
        return true;
    });
}

void HMDT::GUI::StatePropertiesPane::buildIsImpassableField() {
    m_is_impassable_button = addWidget<Gtk::CheckButton>("Is Impassable");

    m_is_impassable_button->signal_toggled().connect([this]() {
        if(m_is_updating_properties) return;

        if(m_state != nullptr) {
            Action::ActionManager::getInstance().doAction(
                NewSetPropertyAction(m_state, impassable,
                                     m_is_impassable_button->get_active()));
        }
    });
}

void HMDT::GUI::StatePropertiesPane::buildProvinceListField() {
    auto* frame = addWidget<Gtk::Frame>();
    m_province_list_window = new Gtk::ScrolledWindow();
    m_province_list_window->set_size_request(-1, 130);

    frame->add(*m_province_list_window);

    m_province_list = new Gtk::ListBox;
    m_province_list->set_selection_mode(Gtk::SELECTION_SINGLE);
    m_province_list->signal_row_selected().connect([](Gtk::ListBoxRow* row) {
        if(auto* province_row = dynamic_cast<ProvinceRow*>(row);
                 province_row != nullptr)
        {
            auto id = province_row->getProvinceID();
            SelectionManager::getInstance().selectProvince(id);
        }
    });

    m_province_list_window->add(*m_province_list);

    m_province_list_window->show_all();
}

void HMDT::GUI::StatePropertiesPane::buildSelectAllProvincesButton() {
    m_select_all_provinces = addWidget<Gtk::Button>("Select All Provinces");

    m_select_all_provinces->signal_clicked().connect([this]() {
        if(m_state != nullptr) {
            // Take a copy first, because clearProvinceSelection will cause
            //  m_state to become null
            auto provinces_copy = m_state->provinces;

            SelectionManager::getInstance().clearProvinceSelection();
            for(auto prov_id : provinces_copy) {
                SelectionManager::getInstance().addProvinceSelection(prov_id);
            }
        }
    });
}

void HMDT::GUI::StatePropertiesPane::buildDeleteStateButton() {
    m_delete_state_button = addWidget<Gtk::Button>("Delete State");
    m_delete_state_button->get_style_context()->add_class(StyleClasses::DESTRUCTIVE_ACTION.data());

    m_delete_state_button->signal_clicked().connect([this]() {
        if(auto opt_project = Driver::getInstance().getProject();
                m_state != nullptr && opt_project)
        {
            auto& map_project = opt_project->get().getMapProject();
            map_project.getStateProject().removeState(m_state->id);

            SelectionManager::getInstance().removeStateSelection(m_state->id);
        }
    });
}

void HMDT::GUI::StatePropertiesPane::addWidgetToParent(Gtk::Widget& widget) {
    m_box.add(widget);
}

/**
 * @brief Sets the sensitivity/enabled state of every field
 *
 * @param enabled
 */
void HMDT::GUI::StatePropertiesPane::setEnabled(bool enabled) {
    m_name_field->set_sensitive(enabled);
    m_manpower_field->set_sensitive(enabled);
    m_category_field->set_sensitive(enabled);
    m_buildings_max_level_factor_field->set_sensitive(enabled);
    m_is_impassable_button->set_sensitive(enabled);
    m_province_list->set_sensitive(enabled);
    m_select_all_provinces->set_sensitive(enabled);
    m_delete_state_button->set_sensitive(enabled);
}

void HMDT::GUI::StatePropertiesPane::setState(State* state, bool is_multiselect)
{
    m_state = state;

    setEnabled(m_state != nullptr && !is_multiselect);

    updateProperties(state, is_multiselect);
}

void HMDT::GUI::StatePropertiesPane::onResize() { }

void HMDT::GUI::StatePropertiesPane::updateProperties(bool is_multiselect) {
    updateProperties(m_state, is_multiselect);
}

/**
 * @brief Will update all of the values stored in every field to the given
 *        province. If prov is nullptr, then the values are changed to defaults.
 *
 * @param prov The province to update the properties to.
 */
void HMDT::GUI::StatePropertiesPane::updateProperties(const State* state,
                                                      bool is_multiselect)
{
    m_is_updating_properties = true;

    if(state == nullptr || is_multiselect) {
        // Set every field to some sort of sane default
        m_name_field->set_text("");
        m_manpower_field->set_text("");
        m_category_field->set_text("");
        m_buildings_max_level_factor_field->set_text(std::to_string(DEFAULT_BUILDINGS_MAX_LEVEL_FACTOR));
        m_is_impassable_button->set_active(false);
    } else {
        // Set every field to state
        m_name_field->set_text(state->name);
        m_manpower_field->set_text(std::to_string(state->manpower));
        m_category_field->set_text(state->category);
        m_buildings_max_level_factor_field->set_text(std::to_string(state->buildings_max_level_factor));
        m_is_impassable_button->set_active(state->impassable);
    }

    updateProvinceListElements(state);

    m_is_updating_properties = false;
}

void HMDT::GUI::StatePropertiesPane::updateProvinceListElements(const State* state)
{
    // Some of this code comes from here: https://stackoverflow.com/a/41388444

    // First we need to clear out every element in the list
    auto children = m_province_list->get_children();
    m_province_list->unselect_all();
    for(Gtk::Widget* child : children) {
        m_province_list->remove(*child);
    }

    // Next only add the new provinces to the list if there are provinces to add
    if(state != nullptr) {
        WRITE_DEBUG("Populating list with ", state->provinces.size(), " provinces.");
        for(ProvinceID prov : state->provinces) {
            auto row = manage(new ProvinceRow(m_province_list, prov));
            m_province_list->append(*row);
            row->show();
        }
    }
}

