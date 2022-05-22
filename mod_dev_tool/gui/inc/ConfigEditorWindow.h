#ifndef CONFIG_EDITOR_WINDOW_H
# define CONFIG_EDITOR_WINDOW_H

# include "gtkmm/box.h"
# include "gtkmm/label.h"
# include "gtkmm/paned.h"
# include "gtkmm/frame.h"
# include "gtkmm/entry.h"
# include "gtkmm/button.h"
# include "gtkmm/window.h"
# include "gtkmm/listbox.h"
# include "gtkmm/separator.h"
# include "gtkmm/linkbutton.h"
# include "gtkmm/scrolledwindow.h"

# include "Preferences.h"

namespace HMDT::GUI {
    class ConfigEditorWindow: public Gtk::Window {
        public:
            ConfigEditorWindow();

        protected:
            /**
             * @brief A named ListBox row
             */
            class NamedRow: public Gtk::ListBoxRow {
                public:
                    NamedRow(const std::string&, uint32_t);
                    NamedRow(const std::string&);
                    ~NamedRow() = default;

                    const std::string& getName() const;

                private:
                    //! The name of this row
                    std::string m_name;

                    //! Container for all elements of this row
                    Gtk::Box m_box;

                    //! The label of this row
                    Gtk::Label m_label;

                    Gtk::Separator m_separator;
            };

            void initWidgets();

            void buildEditorWidget(Gtk::Box&, const std::string&,
                                   const std::string&,
                                   const std::string&,
                                   Preferences::ValueVariant);

        private:
            //! The container for all sub widgets
            Gtk::Box m_box;

            //! The left side of the box
            Gtk::Box m_left;
            
            //! The right side of the box
            Gtk::Frame m_right;

            ////////////////////////////////////////////////////////////////////

            //! The search frame
            Gtk::Frame m_search_frame;

            //! The search box
            Gtk::Box m_search_box;

            //! The label for the search field
            Gtk::Label m_search_label;

            //! The search entry field
            Gtk::Entry m_config_search;

            //! A vector containing all of the search results
            std::vector<Gtk::LinkButton> m_result_links;

            //! The box containing all results
            Gtk::Box m_result_box;

            //! The window where the results are shown
            Gtk::ScrolledWindow m_result_window;

            ////////////////////////////////////////////////////////////////////

            //! The sections label
            Gtk::Label m_sections_label;

            //! The sections frame, contains the list of Sections
            Gtk::Frame m_sections_frame;

            //! The sections window, contains the sections frame
            Gtk::ScrolledWindow m_sections_window;

            //! The list of sections
            Gtk::ListBox m_sections_list;

            ////////////////////////////////////////////////////////////////////

            //! The Groups window, contains the Groups for each section
            std::map<std::string, Gtk::ScrolledWindow> m_groups_windows;

            ////////////////////////////////////////////////////////////////////

            //! Box to hold the save and reset buttons
            Gtk::Box m_save_reset_box;

            //! Saves the current config values
            Gtk::Button m_save_button;

            //! Resets the config values to defaults
            Gtk::Button m_reset_button;

            ////////////////////////////////////////////////////////////////////

            //! Map of paths to functions which update the config editor
            std::map<std::string, std::function<void()>> m_config_updaters;
    };
}

#endif

