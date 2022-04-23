#ifndef MAINWINDOWPROPERTIESPANEPART_H
# define MAINWINDOWPROPERTIESPANEPART_H

# include <memory>

# include "gtkmm/paned.h"
# include "gtkmm/frame.h"

# include "BaseMainWindow.h"

# include "ProvincePropertiesPane.h"
# include "StatePropertiesPane.h"

namespace MapNormalizer::GUI {
    class MainWindowPropertiesPanePart: public virtual BaseMainWindow {
        public:
            MainWindowPropertiesPanePart() = default;

        protected:
            Gtk::Frame* buildPropertiesPane(Gtk::Paned*);

            ProvincePropertiesPane& getProvincePropertiesPane();
            StatePropertiesPane& getStatePropertiesPane();

            const ProvincePropertiesPane& getProvincePropertiesPane() const;
            const StatePropertiesPane& getStatePropertiesPane() const;

        private:
            //! A container holding properties for provinces
            std::unique_ptr<ProvincePropertiesPane> m_province_properties_pane;

            //! A container holding properties for states
            std::unique_ptr<StatePropertiesPane> m_state_properties_pane;
    };
}

#endif

