
#include "ProvinceListWindow.h"

#include "StyleClasses.h"

/**
 * @brief Constructs a new ProvinceRow
 *
 * @param owning_box The ListBox that owns this row
 * @param id The province ID this row is representing
 * @param info A reference to additional information about this row
 */
HMDT::GUI::ProvinceListWindow::ProvinceRow::ProvinceRow(Gtk::ListBox* owning_box,
                                                        ProvinceID id,
                                                        const ProvinceRowInfo& info):
    m_province_id(id),
    m_owning_box(owning_box),
    m_hbox(),
    m_label(info.label_prefix + std::to_string(id)), // Some spaces for left-padding
    m_remove_button(info.remove_button_label),
    m_info(info)
{
    m_remove_button.set_relief(Gtk::RELIEF_NONE);
    if(info.is_destructive) {
        m_remove_button.get_style_context()->add_class(StyleClasses::DESTRUCTIVE_ACTION.data());
    }
    m_remove_button.signal_clicked().connect([this]() {
        if(m_info.callback(m_province_id)) {
            if(m_info.remove_self) {
                // Either way though, remove it from the list
                m_owning_box->remove(*this);
            }
        }
    });

    m_hbox.pack_start(m_label, Gtk::PACK_SHRINK);
    m_hbox.pack_start(m_remove_button, Gtk::PACK_EXPAND_PADDING);
    add(m_hbox);

    show_all_children();
}

/**
 * @brief Gets the ID for the province this row represents
 *
 * @return The ID for the province this row represents
 */
auto HMDT::GUI::ProvinceListWindow::ProvinceRow::getProvinceID() const
    -> ProvinceID
{
    return m_province_id;
}

/**
 * @brief Constructs a new ProvinceListWindow
 *
 * @param callback A function to call whenever a row is clicked on in this list.
 * @param info Additional information to describe how rows should behave
 */
HMDT::GUI::ProvinceListWindow::ProvinceListWindow(const std::function<void(const ProvinceID&)>& callback,
                                                  const ProvinceRowInfo& info):
    m_info(info)
{
    set_size_request(-1, 130);

    m_province_list = new Gtk::ListBox;
    m_province_list->set_selection_mode(Gtk::SELECTION_SINGLE);
    m_province_list->signal_row_selected().connect([callback](Gtk::ListBoxRow* row)
    {
        if(auto* province_row = dynamic_cast<ProvinceRow*>(row);
                 province_row != nullptr)
        {
            auto id = province_row->getProvinceID();

            callback(id);
        }
    });

    add(*m_province_list);
}

/**
 * @brief Sets the list of elements contained in this row
 *
 * @param elements The elements that this row will contain
 */
void HMDT::GUI::ProvinceListWindow::setListElements(const std::set<ProvinceID>& elements) noexcept
{
    // Some of this code comes from here: https://stackoverflow.com/a/41388444

    // First we need to clear out every element in the list
    auto children = m_province_list->get_children();
    m_province_list->unselect_all();
    for(Gtk::Widget* child : children) {
        m_province_list->remove(*child);
    }

    for(ProvinceID prov : elements) {
        auto row = manage(new ProvinceRow(m_province_list, prov, m_info));
        m_province_list->append(*row);
        row->show();
    }
}

/**
 * @brief Whether this window is enabled
 *
 * @param enabled True if this window should be enabled, false if it shouldn't.
 */
void HMDT::GUI::ProvinceListWindow::setListEnabled(bool enabled) noexcept {
    m_province_list->set_sensitive(enabled);
}

