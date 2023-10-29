#ifndef PROVINCE_LIST_WINDOW
# define PROVINCE_LIST_WINDOW

# include <functional>
# include <string>
# include <set>

# include "Types.h"

# include "gtkmm/scrolledwindow.h"
# include "gtkmm/listbox.h"
# include "gtkmm/button.h"
# include "gtkmm/label.h"
# include "gtkmm/box.h"

namespace HMDT::GUI {
    /**
     * @brief Custom scrolled window to display a list of provinces.
     */
    class ProvinceListWindow: public Gtk::ScrolledWindow {
        public:
            /**
             * @brief Info used for a single row.
             */
            struct ProvinceRowInfo {
                //! Prefix appended to the province label
                std::string label_prefix;

                //! Label used for the remove button
                std::string remove_button_label;

                //! Whether the remove button should be displayed with a "Destructive" style
                bool is_destructive;

                //! Whether clicking the remove button should remove the row
                bool remove_self;

                //! Function to be called when the remove button is clicked
                std::function<bool(const ProvinceID&)> callback;
            };

            ProvinceListWindow(const std::function<void(const ProvinceID&)>&,
                               const ProvinceRowInfo&);

            void setListElements(const std::set<ProvinceID>&) noexcept;

            void setListEnabled(bool = true) noexcept;

        protected:
            /**
             * @brief A special ListBox row for representing a single row in the
             *        province list
             */
            class ProvinceRow: public Gtk::ListBoxRow {
                public:
                    ProvinceRow(Gtk::ListBox*, ProvinceID,
                                const ProvinceRowInfo&);
                    ~ProvinceRow() = default;

                    ProvinceID getProvinceID() const;

                    // Each row looks like the following:
                    /////////////////
                    // <LABEL> <X> //
                    /////////////////

                private:
                    //! The province ID
                    ProvinceID m_province_id;

                    //! The ListBox that owns this row
                    Gtk::ListBox* m_owning_box;

                    // The widgets for this particular row

                    //! Main box that stores the row
                    Gtk::Box m_hbox;

                    //! The label of the row
                    Gtk::Label m_label;

                    //! The remove button
                    Gtk::Button m_remove_button;

                    //! Reference for easy access to additional information
                    const ProvinceRowInfo& m_info;
            };

        private:
            //! The actual list of provinces
            Gtk::ListBox* m_province_list;

            //! Additional information for use in rows
            ProvinceRowInfo m_info;
    };
}

#endif

