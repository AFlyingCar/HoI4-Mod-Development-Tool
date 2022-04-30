#ifndef MAINWINDOWDRAWINGAREAPART_H
# define MAINWINDOWDRAWINGAREAPART_H

# include <memory>

# include "BaseMainWindow.h"
# include "IMapDrawingArea.h"

# include "MapDrawingAreaGL.h"
# include "MapDrawingArea.h"

namespace MapNormalizer::GUI {
    class MainWindowDrawingAreaPart: public virtual BaseMainWindow {
        public:
            MainWindowDrawingAreaPart() = default;

            void buildDrawingArea();

            std::shared_ptr<IMapDrawingAreaBase> getDrawingArea();

            void setShouldDrawAdjacencies(bool = true);
            bool shouldDrawAdjacencies() const;

        protected:
            //! The DrawingArea that the map gets rendered to.
            std::shared_ptr<IMapDrawingAreaBase> m_drawing_area;

            //! The box that each DrawingArea is rendered into
            std::shared_ptr<Gtk::Box> m_drawing_box;

            std::shared_ptr<GL::MapDrawingArea> m_gl_drawing_area;
            std::shared_ptr<MapDrawingArea> m_cairo_drawing_area;
    };
}

#endif

