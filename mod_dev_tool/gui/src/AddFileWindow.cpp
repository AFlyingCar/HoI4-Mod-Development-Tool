
#include "AddFileWindow.h"

#include "gtkmm/messagedialog.h"

#include "NativeDialog.h"

#include "Logger.h"

#include "Util.h"
#include "StatusCodes.h"

#include "Item.h"

HMDT::GUI::AddFileWindow::ItemTypeRow::ItemTypeRow(const std::string& name,
                                                   uint32_t font_size,
                                                   const std::string& icon_path):
    ItemTypeRow(name, icon_path)
{
    m_label.set_markup(std::string("<span size=\"") + std::to_string(font_size) + "\">" + name + "</span>");
}

HMDT::GUI::AddFileWindow::ItemTypeRow::ItemTypeRow(const std::string& name,
                                                   const std::string& icon_path):
    m_name(name),
    m_box(Gtk::ORIENTATION_VERTICAL),
    m_icon_label_box(Gtk::ORIENTATION_HORIZONTAL),
    m_icon(),
    m_label(name),
    m_separator(Gtk::ORIENTATION_HORIZONTAL)
{
    m_label.set_xalign(0.0);

    try {
        m_icon.set(Gdk::Pixbuf::create_from_resource(icon_path, 24, 24));
    } catch(...) {
        WRITE_ERROR("Failed to load icon '", icon_path,
                    "'. Using broken-image fallback.");
        m_icon.set_from_icon_name("", Gtk::ICON_SIZE_LARGE_TOOLBAR);
    }
    // m_icon.set_icon_size(Gtk::ICON_SIZE_MENU);

    // Add a label and icon for this row
    m_icon_label_box.add(m_icon);
    m_icon_label_box.add(m_label);

    m_box.add(m_separator);

    m_box.add(m_icon_label_box);

    m_box.show_all();

    add(m_box);
}

const std::string& HMDT::GUI::AddFileWindow::ItemTypeRow::getName() const {
    return m_name;
}

HMDT::GUI::AddFileWindow::AddFileWindow(HMDT::GUI::Window& parent):
    m_parent(parent),
    m_box(Gtk::ORIENTATION_VERTICAL),
    m_paned(),
    m_left_frame(),
    m_right_frame(),
    m_item_types_window(),
    m_item_types_list(),
    m_description_area(),
    m_buttons_box(Gtk::ORIENTATION_HORIZONTAL),
    m_cancel_button("Cancel"),
    m_continue_button("Add")
{
    set_title("Add File");
    set_default_size(662, 440); // TODO

    initWidgets();

    m_box.show_all();
}

void HMDT::GUI::AddFileWindow::initWidgets() {
    add(m_box);

    // Build left side
    {
        // Build the listbox of item types
        buildItemTypesList();

        m_item_types_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

        m_item_types_window.add(m_item_types_list);

        m_item_types_window.show_all();

        m_left_frame.add(m_item_types_window);

        // Make sure that the left frame will take up as much space as possible
        m_left_frame.set_vexpand();

        m_paned.pack1(m_left_frame, true, false);
    }

    // Build right side
    {
        // Set up all properties we want on the description area
        m_description_area.set_line_wrap(true);
        m_description_area.set_line_wrap_mode(Pango::WRAP_WORD_CHAR);
        m_description_area.set_justify(Gtk::JUSTIFY_LEFT);
        m_description_area.set_xalign(0.0);

        // Add it to a special box so that we can ensure that the label is
        //   aligned with the top-left of the right frame
        auto box = manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
        box->pack_start(m_description_title, false, false, 1);
        box->pack_start(m_description_image, false, false, 10);
        box->pack_start(m_description_area, false, false);

        m_right_frame.add(*box);

        m_paned.pack2(m_right_frame, true, false);
    }

    // Set a default position for the handle so that the description doesn't 
    //   default to looking 'squished'
    m_paned.set_position(366); // TODO: Make this a constant

    // Add the pane to the window
    m_box.add(m_paned);

    // Set up button functionality
    m_cancel_button.signal_clicked().connect([this]() {
        close();
    });
    m_continue_button.signal_clicked().connect([this]() -> MaybeVoid {
        RUN_AT_SCOPE_END([this]() { close(); });

        Gtk::ListBoxRow* selected = m_item_types_list.get_selected_row();

        if(auto* row = dynamic_cast<ItemTypeRow*>(selected); row != nullptr) {
            auto maybe_item_type = getItemType(row->getName());
            RETURN_IF_ERROR(maybe_item_type);

            const auto& item_type = maybe_item_type->get();

            // Allocate this on the stack so that it gets automatically cleaned up
            //  when we finish
            NativeDialog::FileDialog dialog("Choose an input file",
                                            NativeDialog::FileDialog::SELECT_FILE);
            // dialog.setDefaultPath() // TODO: Start in the installation directory/Documents

            // Build all specialized filters
            for(auto&& [label, filters] : item_type.filters) {
                dialog.addFilter(label, filters);
            }

            std::string path;
            dialog.addFilter("All files", "")
                  .setAllowsMultipleSelection(item_type.allow_multiselect)
                  .setDecideHandler([&path](const NativeDialog::Dialog& dialog) {
                        auto& fdlg = dynamic_cast<const NativeDialog::FileDialog&>(dialog);
                        path = fdlg.selectedPathes().front();
                  }).show();

            // TODO: Handle the case of allowing multiselect

            if(!path.empty() &&
               IS_FAILURE(addItem(item_type.name, m_parent, std::string(path))))
            {
                Gtk::MessageDialog err_diag("Failed to open file.",
                                            false, Gtk::MESSAGE_ERROR);
                err_diag.run();
            }
        } else {
            WRITE_ERROR("Invalid row selected! Unable to convert selected row to ItemTypeRow.");
        }

        return STATUS_SUCCESS;
    });

    // Add the buttons to the window
    m_buttons_box.pack_end(m_cancel_button, false, false);
    m_buttons_box.pack_end(m_continue_button, false, false);

    m_box.add(m_buttons_box);
}

void HMDT::GUI::AddFileWindow::buildItemTypesList() {
    m_item_types_list.set_selection_mode(Gtk::SELECTION_SINGLE);
    m_item_types_list.signal_row_selected().connect([this](Gtk::ListBoxRow* row)
    {
        if(auto* named_row = dynamic_cast<ItemTypeRow*>(row);
                 named_row != nullptr)
        {
            getItemType(named_row->getName())
                .andThen([this](const ItemType& item_type) {
                    m_description_title.set_markup("<span size=\"14000\"><b>" + item_type.name + "</b></span>");

                    try {
                        m_description_image.set(Gdk::Pixbuf::create_from_resource(item_type.icon, 64, 64));
                    } catch(...) {
                        WRITE_ERROR("Failed to load icon '", item_type.icon,
                                    "'. Using broken-image fallback.");
                        m_description_image.set_from_icon_name("", Gtk::ICON_SIZE_DIALOG);
                    }

                    m_description_area.set_text(item_type.description);
                });
        }
    });

    // Add every item type to the list box
    for(auto&& [name, item_type] : getRegisteredItemTypes()) {
        auto row = manage(new ItemTypeRow(name, 14000, item_type.icon));

        m_item_types_list.append(*row);
        row->show();
    }
}

