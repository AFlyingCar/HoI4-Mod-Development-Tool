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
    class ProvinceListWindow: public Gtk::ScrolledWindow {
        public:
            struct ProvinceRowInfo {
                std::string label_prefix;
                std::string remove_button_label;
                bool is_destructive;
                bool remove_self;
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
                    ProvinceID m_province_id;

                    //! The ListBox that owns this row
                    Gtk::ListBox* m_owning_box;

                    // The widgets for this particular row
                    Gtk::Box m_hbox;
                    Gtk::Label m_label;
                    Gtk::Button m_remove_button;

                    const ProvinceRowInfo& m_info;
            };

        private:
            Gtk::ListBox* m_province_list;

            ProvinceRowInfo m_info;
    };
}

#endif

