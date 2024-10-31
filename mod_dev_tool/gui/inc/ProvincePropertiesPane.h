#ifndef PROVINCE_PROPERTIES_PANE_H
# define PROVINCE_PROPERTIES_PANE_H

# include "WidgetContainer.h"

# include <set>
# include <string>

# include "gtkmm/scrolledwindow.h"
# include "gtkmm/checkbutton.h"
# include "gtkmm/comboboxtext.h"
# include "gtkmm/entry.h"
# include "gtkmm/box.h"

# include "INode.h"

# include "Types.h"
# include "ProvinceListWindow.h"

# include "ProvincePreviewDrawingArea.h"

namespace HMDT::GUI {
    class BaseMainWindow;

    /**
     * @brief The pane where properties of a province are placed into
     */
    class ProvincePropertiesPane: public WidgetContainer {
        public:
            using ValueChangedCallback = std::function<void(const Project::Hierarchy::Key&)>;

            ProvincePropertiesPane(BaseMainWindow&);

            Gtk::ScrolledWindow& getParent();

            void init();

            void setEnabled(bool = true);

            void setProvince(Province*, ProvincePreviewDrawingArea::DataPtr, bool = false);
            Province* getProvince();

            void onResize();

            void updateProperties(bool);

            void onProjectOpened();

            void setCallbackOnValueChanged(const ValueChangedCallback&) noexcept;

        protected:
            virtual void addWidgetToParent(Gtk::Widget&) override;

            void updateMergedListElements(const Province* prov);
            void updateProperties(const Province*, bool);

            void rebuildContinentMenu(const std::set<std::string>&);

            void buildProvincePreviewView();

            void buildIsCoastalField();
            void buildProvinceTypeField();
            void buildTerrainTypeField();
            void buildContinentField();
            void buildStateCreationButton();
            void buildMergeProvincesButton();
            void buildMergedListWindow();

            void setPreview(ProvincePreviewDrawingArea::DataPtr);

        private:
            BaseMainWindow& m_main_window;

            //! The province currently being acted upon
            Province* m_province;

            Gtk::Box m_box;
            Gtk::ScrolledWindow m_parent;

            ProvincePreviewDrawingArea m_preview_area;

            Gtk::CheckButton* m_is_coastal_button;
            Gtk::ComboBoxText* m_provtype_menu;
            Gtk::ComboBoxText* m_terrain_menu;

            Gtk::ComboBoxText* m_continent_menu;
            Gtk::Button* m_add_button;
            Gtk::Button* m_rem_button;


            Gtk::Button* m_create_state_button;

            bool m_is_updating_properties;

            //! Button used to merge two or more provinces together
            Gtk::Button* m_merge_provinces_button;

            //! A list of all provinces that are merged with the currently set one
            ProvinceListWindow* m_merged_list_window;

            //! A callback which is called when a province property gets modified
            ValueChangedCallback m_value_changed_callback;
    };
}

#endif

