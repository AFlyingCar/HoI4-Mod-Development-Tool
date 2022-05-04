
#include "NewProjectDialog.h"

#include "gtkmm/widget.h"

#include "NativeDialog.h"

HMDT::GUI::NewProjectDialog::NewProjectDialog(Gtk::Window& window,
                                                       bool modal):
    Gtk::Dialog("New Project", window, modal),
    m_row(1),
    m_name_field()
{
    Gtk::Bin* bin = reinterpret_cast<Gtk::Bin*>(get_child());

    bin->add(m_grid);

    buildNameField();

    // Put a blank row here to space the fields apart and make the dialog look
    //   nicer
    buildBlankRow();

    buildPathField();

    m_confirm_button = add_button("Create Project", Gtk::RESPONSE_ACCEPT);
    add_button("Cancel", Gtk::RESPONSE_CANCEL);

    m_confirm_button->set_sensitive(false);

    show_all_children();
}

/**
 * @brief Gets the name of the project
 *
 * @return 
 */
std::string HMDT::GUI::NewProjectDialog::getProjectName() {
    return m_name_field.get_text();
}

/**
 * @brief Gets the path for the project
 *
 * @return 
 */
std::string HMDT::GUI::NewProjectDialog::getProjectPath() {
    return m_path_field.get_text();
}

/**
 * @brief Builds the name field for the dialog
 */
void HMDT::GUI::NewProjectDialog::buildNameField() {
    {
        m_grid.attach(*manage(new Gtk::Label("Name: ")), 0, m_row, 1, 1);
        m_grid.attach(m_name_field, 1, m_row, 2, 1);

        // Make sure we update the confirm button if it can be enabled now
        m_name_field.signal_changed().connect([this]() {
            m_confirm_button->set_sensitive(!m_name_field.get_text().empty() &&
                                            !m_path_field.get_text().empty());
        });
    }

    ++m_row;
}

/**
 * @brief Builds the path field for the dialog
 */
void HMDT::GUI::NewProjectDialog::buildPathField() {
    {
        m_grid.attach(*manage(new Gtk::Label("Path: ")), 0, m_row, 1, 1);
        m_grid.attach(m_path_field, 1, m_row, 1, 1);
        // TODO: add a default path?

        m_path_field.set_width_chars(60);

        // Make sure we update the confirm button if it can be enabled now
        m_path_field.signal_changed().connect([this]() {
            m_confirm_button->set_sensitive(!m_name_field.get_text().empty() &&
                                            !m_path_field.get_text().empty());
        });

        Gtk::Button* choose_button = manage(new Gtk::Button("..."));
        m_grid.attach(*choose_button, 2, m_row, 1, 1);

        choose_button->signal_clicked().connect([this]() {
            // Allocate this on the stack so that it gets automatically cleaned up
            //  when we finish
            std::string path;
            NativeDialog::FileDialog dialog("Path to project",
                                            NativeDialog::FileDialog::SELECT_DIR);
            dialog.setAllowsMultipleSelection(false)
                  .setDecideHandler([&path](const NativeDialog::Dialog& dialog) {
                      auto& fdlg = dynamic_cast<const NativeDialog::FileDialog&>(dialog);
                      path = fdlg.selectedPathes().front();
                  }).show();

            if(!path.empty()) {
                m_path_field.set_text(path);

                m_confirm_button->set_sensitive(!m_name_field.get_text().empty() &&
                                                !m_path_field.get_text().empty());
            }
        });
    }
    ++m_row;
}

/**
 * @brief Builds a blank row for spacing
 */
void HMDT::GUI::NewProjectDialog::buildBlankRow() {
    {
        m_grid.attach(*manage(new Gtk::Label("")), 0, m_row, 1, 1);
    }
    ++m_row;
}

