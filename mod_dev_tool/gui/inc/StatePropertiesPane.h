#ifndef STATE_PROPERTIES_PANE_H
# define STATE_PROPERTIES_PANE_H

# include "WidgetContainer.h"

# include "ProvinceListWindow.h"

# include "gtkmm/scrolledwindow.h"
# include "gtkmm/checkbutton.h"
# include "gtkmm/comboboxtext.h"
# include "gtkmm/entry.h"
# include "gtkmm/box.h"
# include "gtkmm/listbox.h"

# include "Types.h"

# include "ConstrainedEntry.h"

namespace HMDT::GUI {
    /**
     * @brief The pane where properties of a state are placed into
     */
    class StatePropertiesPane: public WidgetContainer {
        public:
            StatePropertiesPane();

            Gtk::ScrolledWindow& getParent();

            void init();

            void setEnabled(bool = true);

            void setState(State*, bool = false);

            void onResize();

            void updateProperties(bool);

        protected:
            virtual void addWidgetToParent(Gtk::Widget&) override;

            void updateProperties(const State*, bool);
            void updateProvinceListElements(const State*);

            void buildNameField();
            void buildManpowerField();
            void buildCategoryField();
            void buildBuildingsMaxLevelFactorField();
            void buildIsImpassableField();
            void buildProvinceListField();

            void buildSelectAllProvincesButton();
            void buildDeleteStateButton();

        private:
            //! The state currently being acted upon
            State* m_state;

            Gtk::Box m_box;
            Gtk::ScrolledWindow m_parent;

            Gtk::Entry* m_name_field;     // String field
            ConstrainedEntry* m_manpower_field; // Integer field
            Gtk::Entry* m_category_field; // TODO: Dropdown menu???
            ConstrainedEntry* m_buildings_max_level_factor_field; // Float field
            Gtk::CheckButton* m_is_impassable_button;

            //! The list of all provinces in this state
            ProvinceListWindow* m_province_list_window;

            Gtk::Button* m_select_all_provinces;
            Gtk::Button* m_delete_state_button;

            bool m_is_updating_properties;
    };
}

#endif

