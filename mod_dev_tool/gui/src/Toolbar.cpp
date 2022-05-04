
#include "Toolbar.h"

#include "gtkmm/separatortoolitem.h"

#include "StockIcons.h"

#include "ActionManager.h"
#include "Logger.h"

namespace HMDT::GUI {
    constexpr std::string_view VIEW_SWITCH_ICON = "eye";
}

HMDT::GUI::Toolbar::Toolbar(MainWindowDrawingAreaPart& window):
    m_toolbar_items(),
    m_window(window)
{
}

HMDT::GUI::Toolbar::~Toolbar() {
    for(auto* item : m_toolbar_items) {
        delete item;
    }
}

void HMDT::GUI::Toolbar::init() {
    // Set up styles
    set_toolbar_style(Gtk::TOOLBAR_ICONS);

    // Build new item button
    {
        auto* new_item = createNewToolbarItem<Gtk::ToolButton>("_Add");
        new_item->set_icon_name(StockIcons::NEW.data());
        new_item->set_tooltip_text("Add Item");
        new_item->set_sensitive(false);
    }

    {
        // Build undo button
        {
            m_undo_item = createNewToolbarItem<Gtk::ToolButton>("_Undo");
            m_undo_item->set_icon_name(StockIcons::UNDO.data());
            m_undo_item->signal_clicked().connect([this]() {
                if(!Action::ActionManager::getInstance().undoAction()) {
                    WRITE_WARN("Failed to undo action.");
                }

                updateUndoRedoButtons();
            });
            m_undo_item->set_tooltip_text("Undo");
            m_undo_item->set_sensitive(false);
        }

        // Build redo button
        {
            m_redo_item = createNewToolbarItem<Gtk::ToolButton>("_Redo");
            m_redo_item->set_icon_name(StockIcons::REDO.data());
            m_redo_item->signal_clicked().connect([this]() {
                if(!Action::ActionManager::getInstance().redoAction()) {
                    WRITE_WARN("Failed to redo action.");
                }

                updateUndoRedoButtons();
            });
            m_redo_item->set_tooltip_text("Redo");
            m_redo_item->set_sensitive(false);
        }
    }

    // Build Property Painting Tool button
    {
        auto* property_paint_tool_item = createNewToolbarItem<Gtk::ToolButton>("_Paint");
        property_paint_tool_item->set_icon_name("edit-paste-style"); // TODO: Placeholder icon
        property_paint_tool_item->set_tooltip_text("Property Paint Tool");
        property_paint_tool_item->set_sensitive(false);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Non-editing tools
    createNewSeparator();

    // Build Play button
    {
        auto* play_item = createNewToolbarItem<Gtk::ToolButton>("_PlayHoI");
        play_item->set_icon_name(StockIcons::MEDIA_PLAY.data());
        play_item->set_tooltip_text("Play");
        play_item->set_sensitive(false);
    }

    // Build View Switch button
    {
        using namespace std::string_literals;

        auto* view_switch_item = createNewToolbarItem<Gtk::ToolButton>("_SwitchView");
        view_switch_item->set_icon_name(VIEW_SWITCH_ICON.data());
        view_switch_item->signal_clicked().connect([this, view_switch_item]() {
            auto current_mode = m_window.getDrawingArea()->getViewingMode();

            // NOTE: This should be circular, i.e: Each view should advance to
            //   the next one in the enum, and the last enum should set it back
            //   to the first view.
            IMapDrawingAreaBase::ViewingMode newMode;
            switch(current_mode) {
                case IMapDrawingAreaBase::ViewingMode::PROVINCE_VIEW:
                    newMode = IMapDrawingAreaBase::ViewingMode::STATES_VIEW;
                    break;
                case IMapDrawingAreaBase::ViewingMode::STATES_VIEW:
                    newMode = IMapDrawingAreaBase::ViewingMode::PROVINCE_VIEW;
                    break;
                // No default case here because we want compiler errors if we
                //   add new views and forget to update the switch
            }

            // TODO: We also need to update the MenuBar's "View>Switch Views" menu
            m_window.getDrawingArea()->setViewingMode(newMode);
            view_switch_item->set_tooltip_text("Switch View: "s + std::to_string(newMode));
        });

        // TODO: We're just hardcoding that this is the default, can we come up
        //   with a betterr way of organizing this?
        view_switch_item->set_tooltip_text("Switch View: "s + std::to_string(IMapDrawingAreaBase::ViewingMode::PROVINCE_VIEW));
    }

    for(auto* item : m_toolbar_items) {
        append(*item);
    }
}

void HMDT::GUI::Toolbar::updateUndoRedoButtons() {
    if(Action::ActionManager::getInstance().canUndo()) {
        m_undo_item->set_sensitive(true);
    } else {
        m_undo_item->set_sensitive(false);
    }

    if(Action::ActionManager::getInstance().canRedo()) {
        m_redo_item->set_sensitive(true);
    } else {
        m_redo_item->set_sensitive(false);
    }
}

void HMDT::GUI::Toolbar::createNewSeparator() {
    auto* separator = createNewToolbarItem<Gtk::SeparatorToolItem>();
    separator->set_draw(true);
}

