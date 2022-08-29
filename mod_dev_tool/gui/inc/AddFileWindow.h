#ifndef ADD_FILE_WINDOW_H
# define ADD_FILE_WINDOW_H

# include "gtkmm/box.h"
# include "gtkmm/label.h"
# include "gtkmm/paned.h"
# include "gtkmm/frame.h"
# include "gtkmm/entry.h"
# include "gtkmm/button.h"
# include "gtkmm/window.h"
# include "gtkmm/listbox.h"
# include "gtkmm/separator.h"
# include "gtkmm/scrolledwindow.h"
# include "gtkmm/textview.h"

# include "Window.h"

namespace HMDT::GUI {
    class AddFileWindow: public Gtk::Window {
        public:
            AddFileWindow(HMDT::GUI::Window&);

        protected:
            /**
             * @brief A named ListBox row
             */
            class ItemTypeRow: public Gtk::ListBoxRow {
                public:
                    ItemTypeRow(const std::string&, uint32_t, const std::string&);
                    ItemTypeRow(const std::string&, const std::string&);
                    ~ItemTypeRow() = default;

                    const std::string& getName() const;

                private:
                    //! The name of this row
                    std::string m_name;

                    //! Container for all elements of this row
                    Gtk::Box m_box;

                    //! Container for the icon and label
                    Gtk::Box m_icon_label_box;

                    //! The icon for this item type
                    Gtk::Image m_icon;

                    //! The label of this row
                    Gtk::Label m_label;

                    Gtk::Separator m_separator;
            };

            void initWidgets();

            void buildItemTypesList();

        private:
            //! The parent window
            HMDT::GUI::Window& m_parent;

            //! A box containing every UI element
            Gtk::Box m_box;

            /**
             * @brief The pane to separate out the list of item types from their
             *        descriptions.
             */
            Gtk::Paned m_paned;

            //! The frame around the left pane
            Gtk::Frame m_left_frame;

            //! The frame around the right pane
            Gtk::Frame m_right_frame;

            //! The window where the different item types are displayed
            Gtk::ScrolledWindow m_item_types_window;

            //! The list of item types
            Gtk::ListBox m_item_types_list;

            //! Title above the description area
            Gtk::Label m_description_title;

            //! Image shown above the description
            Gtk::Image m_description_image;

            //! The area where the description for the current item is displayed
            Gtk::Label m_description_area;

            //! A box containing the buttons
            Gtk::Box m_buttons_box;

            //! The button which cancels adding a file
            Gtk::Button m_cancel_button;

            //! The button which proceeds adding a file
            Gtk::Button m_continue_button;
    };
}

#endif

