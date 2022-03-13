
#include "StatePropertiesPane.h"

#include "gtkmm/messagedialog.h"
#include "gtkmm/label.h"
#include "gtkmm/frame.h"

#include "Constants.h"
#include "Logger.h"
#include "Util.h"

#include "Driver.h"

MapNormalizer::GUI::StatePropertiesPane::StatePropertiesPane():
    m_state(nullptr),
    m_box(Gtk::ORIENTATION_VERTICAL)
{ }

Gtk::ScrolledWindow& MapNormalizer::GUI::StatePropertiesPane::getParent() {
    return m_parent;
}

/**
 * @brief Initializes every component of the pane
 */
void MapNormalizer::GUI::StatePropertiesPane::init() {
    // Note: The empty labels are here for spacing purposes so that the fields
    //   aren't too bunched up

    m_parent.set_size_request(MINIMUM_PROPERTIES_PANE_WIDTH, -1);
    m_parent.add(m_box);
    addWidget<Gtk::Label>("");

    buildNameField();
    addWidget<Gtk::Label>("");

    buildManpowerField();
    addWidget<Gtk::Label>("");

    buildCategoryField();
    addWidget<Gtk::Label>("");

    buildBuildingsMaxLevelFactorField();
    addWidget<Gtk::Label>("");

    buildIsImpassableField();
    addWidget<Gtk::Label>("");

    buildProvinceListField();
    addWidget<Gtk::Label>("");

    setEnabled(false);

    m_parent.show_all();
}

void MapNormalizer::GUI::StatePropertiesPane::buildNameField() {
    m_name_field = addWidget<Gtk::Entry>();
    m_name_field->set_placeholder_text("State name...");

    m_name_field->signal_changed().connect([this]() {
        if(m_state != nullptr) {
            m_state->name = m_name_field->get_text();
        }
    });
}

void MapNormalizer::GUI::StatePropertiesPane::buildManpowerField() {
    m_manpower_field = addWidget<ConstrainedEntry>();

    // We aren't allowing '-' because manpower cannot be negative
    m_manpower_field->setAllowedChars("0123456789");
    m_manpower_field->set_placeholder_text("Manpower amount (default:0)");

    m_manpower_field->signal_changed().connect([this]() {
        if(m_state != nullptr) {
            m_state->manpower = std::atoi(m_manpower_field->get_text().c_str());
        }
    });
}

void MapNormalizer::GUI::StatePropertiesPane::buildCategoryField() {
    m_category_field = addWidget<Gtk::Entry>();
    // TODO
    m_category_field->set_sensitive(false);
}

void MapNormalizer::GUI::StatePropertiesPane::buildBuildingsMaxLevelFactorField() {
    m_buildings_max_level_factor_field = addWidget<ConstrainedEntry>();

    m_buildings_max_level_factor_field->setAllowedChars("0123456789.");
    m_buildings_max_level_factor_field->set_placeholder_text("Buildings Max Level Factor (default:1.0)");

    m_buildings_max_level_factor_field->signal_changed().connect([this]() {
        if(m_state != nullptr) {
            m_state->buildings_max_level_factor = std::atof(m_buildings_max_level_factor_field->get_text().c_str());
        }
    });
}

void MapNormalizer::GUI::StatePropertiesPane::buildIsImpassableField() {
    m_is_impassable_button = addWidget<Gtk::CheckButton>("Is Impassable");

    m_is_impassable_button->signal_toggled().connect([this]() {
        if(m_state != nullptr) {
            m_state->impassable = m_is_impassable_button->get_active();
        }
    });
}

void MapNormalizer::GUI::StatePropertiesPane::buildProvinceListField() {
    m_province_list = addWidget<Gtk::ListBox>();
}

void MapNormalizer::GUI::StatePropertiesPane::addWidgetToParent(Gtk::Widget& widget)
{
    m_box.add(widget);
}

/**
 * @brief Sets the sensitivity/enabled state of every field
 *
 * @param enabled
 */
void MapNormalizer::GUI::StatePropertiesPane::setEnabled(bool enabled) {
    m_name_field->set_sensitive(enabled);
    m_manpower_field->set_sensitive(enabled);
    m_category_field->set_sensitive(enabled);
    m_buildings_max_level_factor_field->set_sensitive(enabled);
    m_is_impassable_button->set_sensitive(enabled);
}

void MapNormalizer::GUI::StatePropertiesPane::setState(State* state,
                                                       bool is_multiselect)
{
    m_state = state;

    setEnabled(m_state != nullptr && !is_multiselect);

    updateProperties(state, is_multiselect);
}

void MapNormalizer::GUI::StatePropertiesPane::onResize() { }

/**
 * @brief Will update all of the values stored in every field to the given
 *        province. If prov is nullptr, then the values are changed to defaults.
 *
 * @param prov The province to update the properties to.
 */
void MapNormalizer::GUI::StatePropertiesPane::updateProperties(const State* state,
                                                               bool is_multiselect)
{
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
}

