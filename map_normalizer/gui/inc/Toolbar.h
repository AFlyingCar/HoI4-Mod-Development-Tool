#ifndef TOOLBAR_H
# define TOOLBAR_H

# include <vector>

# include "gtkmm/widget.h"
# include "gtkmm/toolbar.h"
# include "gtkmm/toolitem.h"

# include "IMapDrawingArea.h"

namespace MapNormalizer::GUI {
    class Toolbar: public Gtk::Toolbar {
        public:
            Toolbar();
            ~Toolbar();

            void init();

            void updateUndoRedoButtons();

            void setDrawingArea(std::shared_ptr<IMapDrawingAreaBase>);

        protected:
            template<typename T, typename... Args>
            T* createNewToolbarItem(Args&&... args) {
                T* titem = new T(args...);
                m_toolbar_items.push_back(titem);
                return titem;
            }

            void createNewSeparator();

        private:
            std::vector<Gtk::ToolItem*> m_toolbar_items;

            // MANAGED BY m_toolbar_items
            Gtk::ToolButton* m_undo_item;
            Gtk::ToolButton* m_redo_item;

            std::shared_ptr<IMapDrawingAreaBase> m_drawing_area;
    };
}

#endif

