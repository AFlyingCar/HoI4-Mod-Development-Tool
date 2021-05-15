
#include "ProvincePropertiesPane.h"

#include "gtkmm/messagedialog.h"
#include "gtkmm/label.h"

#include "Constants.h"
#include "Logger.h"

#include "Driver.h"

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
    addWidget<Gtk::Label>("");

    // TODO
#if 0
    buildProvincePreviewView();
    addWidget<Gtk::Label>("");
#endif

    buildIsCoastalField();
    addWidget<Gtk::Label>("");

    buildProvinceTypeField();
    addWidget<Gtk::Label>("");

    buildTerrainTypeField();
    addWidget<Gtk::Label>("");

    buildContinentField();
    addWidget<Gtk::Label>("");

    setEnabled(false);

    m_parent.show_all();
}

void MapNormalizer::GUI::ProvincePropertiesPane::buildIsCoastalField() {
    m_is_coastal_button = addWidget<Gtk::CheckButton>("Is Coastal");

    m_is_coastal_button->signal_toggled().connect([this]() {
        if(m_province != nullptr) {
            m_province->coastal = m_is_coastal_button->get_active();
        }
    });
}

void MapNormalizer::GUI::ProvincePropertiesPane::buildProvinceTypeField() {
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

void MapNormalizer::GUI::ProvincePropertiesPane::buildTerrainTypeField() {
    addWidget<Gtk::Label>("Terrain Type");
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
        if(m_province != nullptr) {
            // TODO: Verify that the active text is a valid terrain type
            m_province->terrain = m_terrain_menu->get_active_text();
        }
    });
}

void MapNormalizer::GUI::ProvincePropertiesPane::buildContinentField() {
    addWidget<Gtk::Label>("Continent");

    m_continent_menu = addWidget<Gtk::ComboBoxText>();
    m_continent_menu->append("None");

    m_continent_menu->signal_changed().connect([this]() {
        if(m_province != nullptr) {
            m_province->continent = m_continent_menu->get_active_text();
        }
    });

    // Add all removable options
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        auto& map_project = opt_project->get().getMapProject();
        const auto& continents = map_project.getContinentList();

        for(auto&& continent : continents) {
            m_continent_menu->append(continent);
        }

        // Add+Remove buttons for continents

        Gtk::Button* add_button;
        Gtk::Button* rem_button;

        Gtk::Box* add_rem_box = addWidget<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);

        add_button = manage(new Gtk::Button("+"));
        rem_button = manage(new Gtk::Button("-"));

        add_rem_box->add(*add_button);
        add_rem_box->add(*rem_button);

        // The remove button only does stuff if there are continents _to_
        //  remove
        rem_button->set_sensitive(!continents.empty());

        add_button->signal_clicked().connect([this, rem_button, &map_project]()
        {
            const auto& continents = map_project.getContinentList();

            Gtk::Dialog add_dialog("Add a continent");
            Gtk::Entry continent_name_entry;
            Gtk::Label entry_label("Name of the new continent:");

            Gtk::Bin* bin = reinterpret_cast<Gtk::Bin*>(add_dialog.get_child());

            bin->add(entry_label);
            bin->add(continent_name_entry);

            auto confirm_button = add_dialog.add_button("Confirm", Gtk::RESPONSE_ACCEPT);
            confirm_button->set_sensitive(false);

            add_dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);

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
                    map_project.addNewContinent(continent_name_entry.get_text());

                    // Rebuild the continent menu here so that they remain
                    //  in the same order as in the internal std::set
                    rebuildContinentMenu(continents);
                    break;
                case Gtk::RESPONSE_CANCEL:
                default:
                    return;
            }

            // Make the remove button active again
            rem_button->set_sensitive(true);
        });

        rem_button->signal_clicked().connect([this, rem_button, &map_project]()
        {
            const auto& continents = map_project.getContinentList();

            Gtk::Dialog rem_dialog("Remove a Continent");
            Gtk::Entry continent_name_entry;
            Gtk::Bin* bin = reinterpret_cast<Gtk::Bin*>(rem_dialog.get_child());

            // Add the Confirm and Cancel buttons to the dialog
            bin->add(continent_name_entry);
            auto confirm_button = rem_dialog.add_button("Confirm", Gtk::RESPONSE_ACCEPT);
            confirm_button->set_sensitive(false);

            rem_dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);

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
                    map_project.removeContinent(continent_name_entry.get_text());

                    rebuildContinentMenu(continents);
                    break;
                case Gtk::RESPONSE_CANCEL:
                default:
                    return;
            }

            // Make the remove button active again
            rem_button->set_sensitive(!continents.empty());
        });
    } else {
        writeError("No project is currently loaded!");
    }
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
    m_continent_menu->set_sensitive(enabled);
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
        m_terrain_menu->set_active_text("unknown");
        m_continent_menu->set_active_text("None");
    } else {
        // Set every field to prov
        m_is_coastal_button->set_active(prov->coastal);
        m_provtype_menu->set_active(static_cast<int>(prov->type) - 1);
        m_terrain_menu->set_active_text(prov->terrain.empty() ? "unknown" : prov->terrain.c_str());
        m_continent_menu->set_active_text(prov->continent.empty() ? "None" : prov->continent.c_str());
    }
}

void MapNormalizer::GUI::ProvincePropertiesPane::rebuildContinentMenu(const std::set<std::string>& continents)
{
    m_continent_menu->remove_all();
    m_continent_menu->append("None");
    for(auto&& c : continents) {
        m_continent_menu->append(c);
    }
}
