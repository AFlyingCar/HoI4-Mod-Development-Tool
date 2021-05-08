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
            //! The current row of the grid being built
            uint32_t m_row;

            //! The grid which holds all internal elements
            Gtk::Grid m_grid;

            //! The text entry field for the project name
            Gtk::Entry m_name_field;

            //! The text entry field for the project path
            Gtk::Entry m_path_field;

            //! The confirmation button
            Gtk::Button* m_confirm_button;
    };
}

#endif

