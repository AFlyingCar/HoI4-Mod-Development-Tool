
#include "ConfigEditorWindow.h"

#include <cstdlib>

#include "gtkmm/checkbutton.h"

#include "Preferences.h"

#include "StyleClasses.h"
#include "ConstrainedEntry.h"

HMDT::GUI::ConfigEditorWindow::NamedRow::NamedRow(const std::string& name,
                                                  uint32_t font_size):
    NamedRow(name)
{
    WRITE_DEBUG("NamedRow(", m_name, ", ", font_size, ")");

    m_label.set_markup(std::string("<span size=\"") + std::to_string(font_size) + "\">" + name + "</span>");
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

/**
 * @brief Initializes all widgets
 */
void HMDT::GUI::ConfigEditorWindow::initWidgets() {
    add(m_box);

    // Make sure that we take up all of the available vertical space
    m_box.set_vexpand();

    // Build the Left side
    {
        // Build a config search field
        {
#if 0
            m_search_frame.add(m_search_box);

            m_config_search.signal_activate().connect([this]() {
                auto&& text = m_config_search.get_text();

            });

            m_search_box.add(m_search_label);
            m_search_box.add(m_config_search);
#endif
        }

        // Build a label to mark the Sections section
        {
            // 16-point font. size is in terms of 1024-th of a point
            m_sections_label.set_markup("<span size=\"16384\"><b>Sections</b></span>");
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
            for(auto&& [sec_name, section] : Preferences::getInstance().getDefaultSections())
            {
                auto row = manage(new NamedRow(sec_name, 14000));

                if(!section.comment.empty()) {
                    row->set_tooltip_text(section.comment);
                }

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
            m_reset_button.signal_clicked().connect([this]() {
                // TODO: Dialog box to warn

                Preferences::getInstance().resetToDefaults();

                for(auto&& [path, updater] : m_config_updaters) {
                    WRITE_DEBUG("Updating config element ", path);
                    updater();
                }
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
        m_left.add(m_sections_label);
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

            Gtk::Box* section_box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 15);

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

                    if(!group.comment.empty()) {
                        header->set_tooltip_text(group.comment);
                    }

                    group_frame->set_label_widget(*header);
                } else {
                    WRITE_DEBUG("Not creating header for group '", grp_name, "'. showTitles=", section.showTitles);
                }

                for(auto&& [config_name, comment_config] : group.configs) {
                    WRITE_DEBUG("Building config \"", config_name, '"');

                    auto& [comment, config] = comment_config;

                    auto path = Preferences::buildValuePath(sec_name, grp_name, config_name);

                    buildEditorWidget(*group_frame_box, path, config_name, comment, config);
                }

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

/**
 * @brief Builds an editor widget for the given value type
 *
 * @param box The box to put the editor widget into
 * @param path The path to the config option
 * @param name The name of the config option
 * @param comment The comment for this config option
 * @param default_value The default value for this config option
 */
void HMDT::GUI::ConfigEditorWindow::buildEditorWidget(Gtk::Box& box,
                                                      const std::string& path,
                                                      const std::string& name,
                                                      const std::string& comment,
                                                      Preferences::ValueVariant default_value)
{
    using namespace std::string_literals;

    // Create a different editor widget depending on the config type expected
    //   here
    std::visit([&, this](auto& value) {
        using T = std::decay_t<decltype(value)>;

        Gtk::Box* config_box = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);

        // Add an extra space before the name to add a bit of padding
        Gtk::Label* lbl_value = new Gtk::Label(" "s + name + " ");
        lbl_value->set_xalign(0.0);

        if(!comment.empty()) {
            lbl_value->set_tooltip_text(comment);
        }

        config_box->add(*lbl_value);

        // Try to find what the preference value is at path.
        //   If one is somehow not set, then just use the default value instead
        T default_value = Preferences::getInstance().getPreferenceValue<T>(path).orElse(value);

        if constexpr(std::is_same_v<T, bool>) {
            // Booleans are controlled with a simple check-button
            Gtk::CheckButton* bool_entry = new Gtk::CheckButton;

            bool_entry->signal_toggled().connect([bool_entry, path]() {
                bool state = bool_entry->get_active();

                if(!Preferences::getInstance().setPreferenceValue(path, state))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to ", state);
                }
            });

            // Set up an updater for this entry
            m_config_updaters[path] = [bool_entry, path]() {
                T new_value = Preferences::getInstance().getPreferenceValue<T>(path).orElse(bool_entry->get_active());

                bool_entry->set_active(new_value);
            };

            bool_entry->set_active(default_value);

            config_box->add(*bool_entry);
        } else if constexpr(std::is_integral_v<T>) {
            // Non-Floating integrals are controlled with a constrained text-entry
            ConstrainedEntry* int_entry = new ConstrainedEntry;
            int_entry->setAllowedChars("0123456789");
            int_entry->set_placeholder_text("default: " + std::to_string(value));

            auto converter = [](const std::string& text) -> T {
                if constexpr(std::is_same_v<T, int64_t>) {
                    return std::strtoll(text.c_str(), nullptr, 10);
                } else if constexpr(std::is_same_v<T, uint64_t>) {
                    return std::strtoull(text.c_str(), nullptr, 10);
                } else {
                    // Make sure that we fail for all other integral types
                    static_assert(alwaysFalse<T>, "Unrecognized integral type");
                }
            };

            // When activated, we need to determine what type of strto* function
            //   to call.
            int_entry->signal_activate().connect([int_entry, path, converter]()
            {
                T set_value = converter(int_entry->get_text());

                if(!Preferences::getInstance().setPreferenceValue(path, set_value))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to ", set_value);
                }
            });

            // Leaving the text field will _also_ set the value
            int_entry->signal_focus_out_event().connect([int_entry](GdkEventFocus*)
            {
                int_entry->activate();
                return true;
            });

            // Set up an updater for this entry
            m_config_updaters[path] = [int_entry, path, converter]() {
                T new_value = Preferences::getInstance().getPreferenceValue<T>(path).orElse(converter(int_entry->get_text()));

                int_entry->set_text(std::to_string(new_value));
            };

            // Set default value
            int_entry->set_text(std::to_string(default_value));

            config_box->add(*int_entry);
        } else if constexpr(std::is_floating_point_v<T>) {
            // Floating numbers are controlled with a constrained text-entry
            ConstrainedEntry* float_entry = new ConstrainedEntry;
            float_entry->setAllowedChars("0123456789.");
            float_entry->set_placeholder_text("default: " + std::to_string(value));

            auto converter  = [](const std::string& text) {
                if constexpr(std::is_same_v<T, double>) {
                    return std::strtod(text.c_str(), nullptr);
                } else if constexpr(std::is_same_v<T, float>) {
                    // We don't currently support floats in Preferences, but we
                    //   may as well support it here type-wise just in case
                    return std::strtof(text.c_str(), nullptr);
                } else {
                    // Make sure that we fail for all other integral types
                    static_assert(alwaysFalse<T>, "Unrecognized floating type");
                }
            };

            // When activated, we need to determine what type of strto* function
            //   to call.
            float_entry->signal_activate().connect([float_entry, path, converter]()
            {
                T set_value = converter(float_entry->get_text());

                if(!Preferences::getInstance().setPreferenceValue(path, set_value))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to ", set_value);
                }
            });

            // Leaving the text field will _also_ set the value
            float_entry->signal_focus_out_event().connect([float_entry](GdkEventFocus*)
            {
                float_entry->activate();
                return true;
            });

            // Set up an updater for this entry
            m_config_updaters[path] = [float_entry, path, converter]() {
                T new_value = Preferences::getInstance().getPreferenceValue<T>(path).orElse(converter(float_entry->get_text()));

                float_entry->set_text(std::to_string(new_value));
            };

            // Set default value
            float_entry->set_text(std::to_string(default_value));

            config_box->add(*float_entry);

        } else if constexpr(std::is_same_v<T, std::string>) {
            // Normal text is just controlled with a regular text-entry.
            Gtk::Entry* text_entry = new Gtk::Entry();
            text_entry->set_placeholder_text("default: " + value);

            text_entry->signal_activate().connect([text_entry, path]() {
                if(!Preferences::getInstance().setPreferenceValue(path,
                                                                  text_entry->get_text()))
                {
                    WRITE_ERROR("Failed to set preference value ", path, " to '", text_entry->get_text(), '\'');
                }
            });

            // Leaving the text field will _also_ set the value
            text_entry->signal_focus_out_event().connect([text_entry](GdkEventFocus*)
            {
                text_entry->activate();
                return true;
            });

            // Set up an updater for this entry
            m_config_updaters[path] = [text_entry, path]() {
                T new_value = Preferences::getInstance().getPreferenceValue<T>(path).orElse(text_entry->get_text());

                text_entry->set_text(new_value);
            };

            // Set default value
            text_entry->set_text(default_value);

            config_box->add(*text_entry);
        } else {
            // Make sure we fail to compile if new types were added without also
            //   creating a corresponding config editor here
            static_assert(alwaysFalse<T>,
                          "Unrecognized type for Preferences Value.");
        }

        // Get some minimum spacing between elements to prevent things
        //   from looking too cramped
        box.pack_start(*config_box, false, false, 8);
    }, default_value);
}

