
#include "NewProjectDialog.h"

#include "gtkmm/widget.h"
#include "gtkmm/filechooserdialog.h"

MapNormalizer::GUI::NewProjectDialog::NewProjectDialog(Gtk::Window& window,
                                                       bool modal):
    Gtk::Dialog("New Project", window, modal),
    m_row(1),
    m_name_field()
{
    Gtk::Bin* bin = reinterpret_cast<Gtk::Bin*>(get_child());

    bin->add(m_grid);

    buildNameField();

    buildBlankRow();

    buildPathField();

    m_confirm_button = add_button("Create Project", Gtk::RESPONSE_ACCEPT);
    add_button("Cancel", Gtk::RESPONSE_CANCEL);

    m_confirm_button->set_sensitive(false);

    show_all_children();
}

std::string MapNormalizer::GUI::NewProjectDialog::getProjectName() {
    return m_name_field.get_text();
}

std::string MapNormalizer::GUI::NewProjectDialog::getProjectPath() {
    return m_path_field.get_text();
}

void MapNormalizer::GUI::NewProjectDialog::buildNameField() {
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

void MapNormalizer::GUI::NewProjectDialog::buildPathField() {
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
            Gtk::FileChooserDialog dialog(*this, "Path to project",
                                          Gtk::FILE_CHOOSER_ACTION_CREATE_FOLDER);
            dialog.set_select_multiple(false);

            dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
            dialog.add_button("Select", Gtk::RESPONSE_ACCEPT);

            const int result = dialog.run();
            switch(result) {
                case Gtk::RESPONSE_ACCEPT:
                    dialog.hide(); // Hide ourselves immediately

                    m_path_field.set_text(dialog.get_filename());

                    m_confirm_button->set_sensitive(!m_name_field.get_text().empty() &&
                                                    !m_path_field.get_text().empty());
                    break;
                case Gtk::RESPONSE_CANCEL:
                case Gtk::RESPONSE_DELETE_EVENT:
                default:
                    return;
            }
        });
    }
    ++m_row;
}

void MapNormalizer::GUI::NewProjectDialog::buildBlankRow() {
    {
        m_grid.attach(*manage(new Gtk::Label("")), 0, m_row, 1, 1);
    }
    ++m_row;
}

