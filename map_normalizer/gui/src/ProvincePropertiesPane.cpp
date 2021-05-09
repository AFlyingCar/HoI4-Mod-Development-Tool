
#include "ProvincePropertiesPane.h"

#include "gtkmm/messagedialog.h"
#include "gtkmm/label.h"

#include "Constants.h"
#include "Logger.h"

MapNormalizer::GUI::ProvincePropertiesPane::ProvincePropertiesPane():
    m_province(nullptr),
    m_box(Gtk::ORIENTATION_VERTICAL)
{ }

Gtk::ScrolledWindow& MapNormalizer::GUI::ProvincePropertiesPane::getParent() {
    return m_parent;
}

void MapNormalizer::GUI::ProvincePropertiesPane::init() {
    m_parent.set_size_request(MINIMUM_PROPERTIES_PANE_WIDTH, -1);
    m_parent.add(m_box);

    // Is Coastal
    {
        m_is_coastal_button = addWidget<Gtk::CheckButton>("Is Coastal");

        m_is_coastal_button->signal_toggled().connect([this]() {
            if(m_province != nullptr) {
                m_province->coastal = m_is_coastal_button->get_active();
            }
        });
    }

    // Province Type
    addWidget<Gtk::Label>("");
    {
        addWidget<Gtk::Label>("Province Type");

        m_provtype_menu = addWidget<Gtk::ComboBoxText>();

        m_provtype_menu->append("Land");
        m_provtype_menu->append("Sea");
        m_provtype_menu->append("Lake");

        m_provtype_menu->set_active(0);
        m_provtype_menu->signal_changed().connect([this]() {
            if(m_province != nullptr) {
                // Only set the province type if it is set to a valid one
                if(auto current = m_provtype_menu->get_active_row_number();
                        current != -1)
                {
                    m_province->type = static_cast<ProvinceType>(current + 1);
                } else {
                    writeError("Province type somehow set to an invalid index: ",
                               m_provtype_menu->get_active_row_number());
                }
            }
        });
    }

    // Terrain Type
    addWidget<Gtk::Label>("");
    {
        addWidget<Gtk::Label>("Terrain Type");
        m_terrain_menu = addWidget<Gtk::ComboBoxText>();

        // TODO: Add options, how do we know which terrain types are valid?

        m_terrain_menu->set_active(0);
        m_terrain_menu->signal_changed().connect([this]() {
            if(m_province != nullptr) {
                // TODO: Verify that the active text is a valid terrain type
                m_province->terrain = m_terrain_menu->get_active_text();
            }
        });
    }

    // Continent
    addWidget<Gtk::Label>("");
    {
        addWidget<Gtk::Label>("Continent");

        m_continent_entry = addWidget<Gtk::Entry>();
        m_continent_entry->signal_activate().connect([this]() {
            if(m_province != nullptr) {
                auto data = m_continent_entry->get_text();
                // Validate input, only allow positive numbers
                if(data.find_first_not_of("0123456789") != std::string::npos) {
                    Gtk::MessageDialog dialog("Invalid input, continents can only be positive numbers.",
                                              false, Gtk::MESSAGE_ERROR);
                    dialog.run();
                    return;
                }

                m_province->continent = std::atoi(data.c_str());

                // TODO: We should "ungrab"/"release" focus from this widget.
                //  ... How do we actually do that?
            }
        });
    }

    setEnabled(false);

    m_parent.show_all();
}

void MapNormalizer::GUI::ProvincePropertiesPane::addWidgetToParent(Gtk::Widget& widget)
{
    m_box.add(widget);
}

/**
 * @brief Sets the sensitivity/enabled state of every field
 *
 * @param enabled
 */
void MapNormalizer::GUI::ProvincePropertiesPane::setEnabled(bool enabled) {
    m_is_coastal_button->set_sensitive(enabled);
    m_provtype_menu->set_sensitive(enabled);
    m_terrain_menu->set_sensitive(enabled);
    m_continent_entry->set_sensitive(enabled);
}

void MapNormalizer::GUI::ProvincePropertiesPane::setProvince(Province* prov) {
    m_province = prov;

    setEnabled(m_province != nullptr);

    updateProperties(prov);
}

void MapNormalizer::GUI::ProvincePropertiesPane::updateProperties(const Province* prov)
{
    if(prov == nullptr) {
        // Set every field to some sort of sane default
        m_is_coastal_button->set_active(false);
        m_provtype_menu->set_active(0);
        m_terrain_menu->set_active(0);
        m_continent_entry->set_text("");
    } else {
        // Set every field to prov
        m_is_coastal_button->set_active(prov->coastal);
        m_provtype_menu->set_active(static_cast<int>(prov->type) - 1);
        // TODO
        // m_terrain_menu->set_selected(0);
        m_continent_entry->set_text(std::to_string(prov->continent));
    }
}

