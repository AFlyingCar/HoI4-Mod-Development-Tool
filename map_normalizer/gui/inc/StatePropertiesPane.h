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

            void updateProperties(bool);

        protected:
            /**
             * @brief A special ListBox row for representing a single row in the
             *        province list
             */
            class ProvinceRow: public Gtk::ListBoxRow {
                public:
                    ProvinceRow(Gtk::ListBox*, ProvinceID);
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
            };

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

            Gtk::ScrolledWindow* m_province_list_window;
            Gtk::ListBox* m_province_list;

            Gtk::Button* m_select_all_provinces;
            Gtk::Button* m_delete_state_button;
    };
}

#endif

