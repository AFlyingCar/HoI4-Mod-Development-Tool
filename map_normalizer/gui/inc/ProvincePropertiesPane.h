#ifndef PROVINCE_PROPERTIES_PANE_H
# define PROVINCE_PROPERTIES_PANE_H

# include "WidgetContainer.h"

# include "gtkmm/scrolledwindow.h"
# include "gtkmm/checkbutton.h"
# include "gtkmm/comboboxtext.h"
# include "gtkmm/entry.h"
# include "gtkmm/box.h"

# include "Types.h"

namespace MapNormalizer::GUI {
    class ProvincePropertiesPane: public WidgetContainer {
        public:
            ProvincePropertiesPane();

            Gtk::ScrolledWindow& getParent();

            void init();

            void setEnabled(bool = true);

            void setProvince(Province*);

        protected:
            virtual void addWidgetToParent(Gtk::Widget&) override;

            void updateProperties(const Province*);

        private:
            Province* m_province;

            Gtk::Box m_box;
            Gtk::ScrolledWindow m_parent;

            Gtk::CheckButton* m_is_coastal_button;
            Gtk::ComboBoxText* m_provtype_menu;
            Gtk::ComboBoxText* m_terrain_menu;
            Gtk::Entry* m_continent_entry;
    };
}

#endif

