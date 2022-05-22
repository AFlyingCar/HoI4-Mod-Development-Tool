
#include "ConfigEditorWindow.h"

#include <cstdlib>

#include "gtkmm/checkbutton.h"

#include "Preferences.h"

#include "StyleClasses.h"
#include "ConstrainedEntry.h"

HMDT::GUI::ConfigEditorWindow::NamedRow::NamedRow(const std::string& name,
                                                  uint32_t font_size):
    m_name(name),
    m_box(Gtk::ORIENTATION_VERTICAL),
    m_label(),
    m_separator(Gtk::ORIENTATION_HORIZONTAL)
{
    WRITE_DEBUG("NamedRow(", m_name, ", ", font_size, ")");

    m_label.set_markup(std::string("<span size=\"") + std::to_string(font_size) + "\">" + name + "</span>");

    m_label.set_xalign(0.0);

    // Add a label for this row
    m_box.add(m_label);
    m_box.add(m_separator);

    m_box.show_all();

    add(m_box);
}

HMDT::GUI::ConfigEditorWindow::NamedRow::NamedRow(const std::string& name):
    m_name(name),
    m_box(Gtk::ORIENTATION_VERTICAL),
    m_label(name),
    m_separator(Gtk::ORIENTATION_HORIZONTAL)
{
    WRITE_DEBUG("NamedRow(", m_name, ")");

    m_label.set_xalign(0.0);

    // Add a label for this row
    m_box.add(m_label);
    m_box.add(m_separator);

    m_box.show_all();

    add(m_box);
}

const std::string& HMDT::GUI::ConfigEditorWindow::NamedRow::getName() const {
    return m_name;
}

HMDT::GUI::ConfigEditorWindow::ConfigEditorWindow():
    m_box(Gtk::ORIENTATION_HORIZONTAL),
    m_left(Gtk::ORIENTATION_VERTICAL),
    m_search_label("Search:"),
    m_save_reset_box(Gtk::ORIENTATION_HORIZONTAL),
    m_save_button("Save Preferences"),
    m_reset_button("Reset to Defaults")
{
    set_title("Config Editor Window");
    set_default_size(662, 440);

    initWidgets();

}

void HMDT::GUI::ConfigEditorWindow::initWidgets() {
    add(m_box);

    // Make sure that we take up all of the available vertical space
    m_box.set_vexpand();

    // Build the Left side
    {
        // Build a config search field
        {
            m_search_frame.add(m_search_box);

            m_config_search.signal_changed().connect([]() {
            });

            m_search_box.add(m_search_label);
            m_search_box.add(m_config_search);
        }

        // Now list all of the sections
        {
            m_sections_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

            m_sections_list.set_selection_mode(Gtk::SELECTION_SINGLE);
            m_sections_list.signal_row_selected().connect([this](Gtk::ListBoxRow* row)
            {
                if(auto* named_row = dynamic_cast<NamedRow*>(row);
                         named_row != nullptr)
                {
                    m_right.remove();
                    m_right.add(m_groups_windows.at(named_row->getName()));
                }
            });

            // Add every defined section name to the listbox
            for(auto&& [sec_name, _] : Preferences::getInstance().getDefaultSections())
            {
                auto row = manage(new NamedRow(sec_name, 14000));
                m_sections_list.append(*row);
                row->show();
            }

            m_sections_window.add(m_sections_list);

            m_sections_window.show_all();

            // Make the minimum size just a bit bigger than its default
            m_sections_window.set_min_content_width(m_sections_window.get_allocation().get_width() + 100);

            // Now add the scrolledwindow to the frame
            m_sections_frame.add(m_sections_window);

            // Make sure that the sections frame will take up as much space as
            //   possible
            m_sections_frame.set_vexpand();
        }

        // Now build out the save and reset buttons
        {
            // m_reset_button.set_relief(Gtk::RELIEF_NONE);
            m_reset_button.get_style_context()->add_class(StyleClasses::DESTRUCTIVE_ACTION.data());
            m_reset_button.signal_clicked().connect([]() {
                Preferences::getInstance().resetToDefaults();

                // TODO: Update all fields
            });

            // m_save_button.set_relief(Gtk::RELIEF_NONE);
            m_save_button.get_style_context()->add_class(StyleClasses::SUGGESTED_ACTION.data());
            m_save_button.signal_clicked().connect([]() {
                if(!Preferences::getInstance().writeToFile(true /* pretty */)) {
                    WRITE_ERROR("Failed to write config options to file.");
                }
            });

            m_save_reset_box.add(m_save_button);
            m_save_reset_box.add(m_reset_button);
        }

        // Now add the search bar and sections to the left-side
        m_left.add(m_search_frame);
        m_left.add(m_sections_frame);

        m_left.add(m_save_reset_box);

        m_box.add(m_left);
    }

    // Build each Groups side
    {
        WRITE_DEBUG("Building Groups Windows.");
        // Build out each set of groups per Section
        for(auto&& [sec_name, section] : Preferences::getInstance().getDefaultSections())
        {
            WRITE_DEBUG("Building Groups window for Section \"", sec_name, '"');

            Gtk::Box* section_box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);

            // Get the ScrolledWindow and Box for this section's groups
            auto& swindow = m_groups_windows[sec_name];

            swindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

            // TODO: Should properties be settable here?

            // Add every group 
            for(auto&& [grp_name, group] : section.groups) {
                WRITE_DEBUG("Building frame for Group \"", grp_name, '"');

                Gtk::Frame* group_frame = new Gtk::Frame;
                Gtk::Box* group_frame_box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);

                // Only show the title if showTitles is true and the title isn't
                //   empty or '_'
                if(section.showTitles && grp_name != "_" && !grp_name.empty()) {
                    WRITE_DEBUG("Creating header for group '", grp_name, "'.");

                    Gtk::Label* header = new Gtk::Label;
                    // TODO: Better way of marking that this is a header
                    header->set_markup(std::string("<b>") + grp_name + "</b>");

                    group_frame->set_label_widget(*header);
                } else {
                    WRITE_DEBUG("Not creating header for group '", grp_name, "'. showTitles=", section.showTitles);
                }

                // Add an empty label for spacing
                group_frame_box->add(*new Gtk::Label);

                for(auto&& [config_name, comment_config] : group.configs) {
                    WRITE_DEBUG("Building config \"", config_name, '"');

                    auto& [comment, config] = comment_config;

                    auto path = Preferences::buildValuePath(sec_name, grp_name, config_name);

                    buildEditorWidget(*group_frame_box, path, config_name, comment, config);
                }

                // Add an empty label for spacing
                group_frame_box->add(*new Gtk::Label);

                group_frame->add(*group_frame_box);
                section_box->add(*group_frame);
                group_frame->show_all();
            }

            section_box->show_all();

            swindow.add(*section_box);
            swindow.show_all();
        }

        auto&& [first_sec_name, _] = *Preferences::getInstance().getDefaultSections().begin();
        WRITE_DEBUG("Setting default section to ", first_sec_name);
        m_right.add(m_groups_windows.at(first_sec_name));
        m_right.set_hexpand();

        m_box.add(m_right);
    }
}

void HMDT::GUI::ConfigEditorWindow::buildEditorWidget(Gtk::Box& box,
                                                      const std::string& path,
                                                      const std::string& name,
                                                      const std::string& comment,
                                                      Preferences::ValueVariant default_value)
{
    std::visit([&](auto& value) {
        using T = std::decay_t<decltype(value)>;

        Gtk::Box* config_box = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);

        Gtk::Label* lbl_value = new Gtk::Label(name + " ");
        lbl_value->set_xalign(0.0);

        config_box->add(*lbl_value);

        T default_value = Preferences::getInstance().getPreferenceValue<T>(path).orElse(value);

        if constexpr(std::is_same_v<T, bool>) {
            Gtk::CheckButton* bool_entry = new Gtk::CheckButton;

            bool_entry->signal_toggled().connect([bool_entry, path]() {
                bool state = bool_entry->get_active();

                if(!Preferences::getInstance().setPreferenceValue(path, state))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to ", state);
                }
            });

            bool_entry->set_active(default_value);

            config_box->add(*bool_entry);
        } else if constexpr(std::is_integral_v<T>) {
            ConstrainedEntry* int_entry = new ConstrainedEntry;
            int_entry->setAllowedChars("0123456789");
            int_entry->set_placeholder_text("default: " + std::to_string(value));

            int_entry->signal_activate().connect([int_entry, path]() {
                T set_value;
                if constexpr(std::is_same_v<T, int64_t>) {
                    set_value = std::strtoll(int_entry->get_text().c_str(), nullptr, 10);
                } else if constexpr(std::is_same_v<T, uint64_t>) {
                    set_value = std::strtoull(int_entry->get_text().c_str(), nullptr, 10);
                } else {
                    // Make sure that we fail for all other integral types
                    static_assert(alwaysFalse<T>, "Unrecognized integral type");
                }

                if(!Preferences::getInstance().setPreferenceValue(path, set_value))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to ", set_value);
                }
            });

            int_entry->signal_focus_out_event().connect([int_entry](GdkEventFocus*)
            {
                int_entry->activate();
                return true;
            });

            // Set default value
            int_entry->set_text(std::to_string(default_value));

            config_box->add(*int_entry);
        } else if constexpr(std::is_floating_point_v<T>) {
            ConstrainedEntry* float_entry = new ConstrainedEntry;
            float_entry->setAllowedChars("0123456789.");
            float_entry->set_placeholder_text("default: " + std::to_string(value));

            float_entry->signal_activate().connect([float_entry, path]() {
                T set_value;
                if constexpr(std::is_same_v<T, double>) {
                    set_value = std::strtod(float_entry->get_text().c_str(), nullptr);
                } else if constexpr(std::is_same_v<T, float>) {
                    set_value = std::strtof(float_entry->get_text().c_str(), nullptr);
                } else {
                    // Make sure that we fail for all other integral types
                    static_assert(alwaysFalse<T>, "Unrecognized floating type");
                }

                if(!Preferences::getInstance().setPreferenceValue(path, set_value))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to ", set_value);
                }
            });

            float_entry->signal_focus_out_event().connect([float_entry](GdkEventFocus*)
            {
                float_entry->activate();
                return true;
            });

            // Set default value
            float_entry->set_text(std::to_string(default_value));

            config_box->add(*float_entry);

        } else if constexpr(std::is_same_v<T, std::string>) {
            Gtk::Entry* text_entry = new Gtk::Entry();
            text_entry->set_placeholder_text("default: " + value);

            text_entry->signal_activate().connect([text_entry, path]() {
                if(!Preferences::getInstance().setPreferenceValue(path,
                                                                  text_entry->get_text()))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to '", text_entry->get_text(), '\'');
                }
            });

            text_entry->signal_focus_out_event().connect([text_entry](GdkEventFocus*)
            {
                text_entry->activate();
                return true;
            });

            // Set default value
            text_entry->set_text(default_value);

            config_box->add(*text_entry);
        } else {
            static_assert(alwaysFalse<T>,
                          "Unrecognized type for Preferences Value.");
        }

        box.add(*config_box);
    }, default_value);
}

