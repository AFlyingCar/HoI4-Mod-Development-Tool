/**
 * @file IRenderingView.h
 *
 * @brief Defines the IRenderingView class
 *
 * @details Each *RenderingView essentially is used to represent an OpenGL
 *          "scene". To define a new scene, inherit from this class and add an
 *          initialization for it to MapDrawingAreaGL::init().
 */

#ifndef IRENDERINGVIEW_H
# define IRENDERINGVIEW_H

# include <memory>
# include <vector>
# include <optional>

# include "MapData.h"
# include "IMapDrawingArea.h" // SelectionInfo

namespace HMDT::GUI::GL {
    class Program;
    class MapDrawingArea;

    /**
     * @brief Base interface for implementing a rendering view
     */
    class IRenderingView {
        public:
            using ProgramList = std::vector<std::reference_wrapper<Program>>;

            IRenderingView() = default;
            virtual ~IRenderingView() = default;
            IRenderingView(IRenderingView&&) = default;

            IRenderingView(const IRenderingView&) = delete;
            IRenderingView& operator=(const IRenderingView&) = delete;

            /**
             * @brief Initializes this IRenderingView
             */
            virtual void init() = 0;

            virtual void onMapDataChanged(std::shared_ptr<const MapData>) = 0;
            virtual void onSelectionChanged(std::optional<IMapDrawingAreaBase::SelectionInfo>) { };

            virtual void beginRender() = 0;
            virtual void render() = 0;
            virtual void endRender() = 0;

            virtual ProgramList getPrograms() = 0;

        protected:
            // Mark that the GL::MapDrawingArea is a friend so it can set
            //  m_owning_gl_drawing_area for us
            friend class MapDrawingArea;

            const MapDrawingArea* getOwningGLDrawingArea() const {
                return m_owning_gl_drawing_area;
            }

        private:
            const MapDrawingArea* m_owning_gl_drawing_area;
    };
}

#endif

