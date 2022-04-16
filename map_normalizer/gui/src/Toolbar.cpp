
#include "Toolbar.h"

#include "gtkmm/separatortoolitem.h"

#include "StockIcons.h"

#include "ActionManager.h"
#include "Logger.h"

MapNormalizer::GUI::Toolbar::Toolbar():
    m_toolbar_items()
{
}

MapNormalizer::GUI::Toolbar::~Toolbar() {
    for(auto* item : m_toolbar_items) {
        delete item;
    }
}

void MapNormalizer::GUI::Toolbar::init() {
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

    for(auto* item : m_toolbar_items) {
        append(*item);
    }
}

void MapNormalizer::GUI::Toolbar::updateUndoRedoButtons() {
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

void MapNormalizer::GUI::Toolbar::createNewSeparator() {
    auto* separator = createNewToolbarItem<Gtk::SeparatorToolItem>();
    separator->set_draw(true);
}

