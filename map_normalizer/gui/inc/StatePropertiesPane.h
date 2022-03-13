#ifndef STATE_PROPERTIES_PANE_H
# define STATE_PROPERTIES_PANE_H

# include "WidgetContainer.h"

# include "gtkmm/scrolledwindow.h"
# include "gtkmm/checkbutton.h"
# include "gtkmm/comboboxtext.h"
# include "gtkmm/entry.h"
# include "gtkmm/box.h"
# include "gtkmm/listbox.h"

# include "Types.h"

# include "ConstrainedEntry.h"

namespace MapNormalizer::GUI {
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

        protected:
            virtual void addWidgetToParent(Gtk::Widget&) override;

            void updateProperties(const State*, bool);

            void buildNameField();
            void buildManpowerField();
            void buildCategoryField();
            void buildBuildingsMaxLevelFactorField();
            void buildIsImpassableField();
            void buildProvinceListField();
            // void buildAddProvinceButtonField();

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
            Gtk::ListBox* m_province_list;
            // TODO: Province-list field? What should this be?
            //  Thoughts: List with '-' button on each element, so you can select
            //   the province by clicking on it (call into normal select/multiselect code), or remove it from this state with the - button
            // Gtk::Button* m_add_province_button; // Button for adding selected provinces to this state?
                                                   //  Maybe instead for combining multiple states together?
    };
}

#endif

