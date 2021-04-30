#ifndef NEWPROJECT_DIALOG_H
# define NEWPROJECT_DIALOG_H

# include <string>

# include "gtkmm/window.h"
# include "gtkmm/dialog.h"
# include "gtkmm/bin.h"
# include "gtkmm/entry.h"
# include "gtkmm/button.h"
# include "gtkmm/grid.h"

namespace MapNormalizer::GUI {
    /**
     * @brief A dialog box which asks for all the information to make a new project.
     */
    class NewProjectDialog: public Gtk::Dialog {
        public:
            NewProjectDialog(Gtk::Window&, bool = false);

            std::string getProjectName();
            std::string getProjectPath();

        protected:
            void buildNameField();
            void buildPathField();

            void buildBlankRow();

        private:
            uint32_t m_row;

            Gtk::Grid m_grid;

            Gtk::Entry m_name_field;
            Gtk::Entry m_path_field;

            Gtk::Button* m_confirm_button;
    };
}

#endif

