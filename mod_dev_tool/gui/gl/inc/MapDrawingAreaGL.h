/**
 * @file MapDrawingAreaGL.h
 *
 * @brief Defines the OpenGL map drawing area
 */

#ifndef MAPDRAWINGAREAGL_H
# define MAPDRAWINGAREAGL_H

# include <memory>
# include <vector>
# include <map>

# include <gtkmm/glarea.h>
# include <glm/vec4.hpp>
# include <glm/mat4x4.hpp>

# include "IMapDrawingArea.h"
# include "Program.h"
# include "Texture.h"
# include "IRenderingView.h"

namespace HMDT::GUI::GL {
    /**
     * @brief A MapDrawingArea using OpenGL
     */
    class MapDrawingArea: public IMapDrawingArea<Gtk::GLArea> {
        public:
            MapDrawingArea();
            virtual ~MapDrawingArea();

            virtual void queueDraw() override;

            glm::mat4 getProjection() const;
            glm::mat4 getTransformation() const;

        protected:
            virtual bool on_render(const Glib::RefPtr<Gdk::GLContext>&) override;
            virtual void on_unrealize() override;
            virtual bool on_button_press_event(GdkEventButton*) override;

            virtual void init() override;
            virtual void onZoom() override;
            virtual void onViewingModeChange(ViewingMode) override { };
            virtual void onSetData(std::shared_ptr<const MapData>) override;
            virtual void onShow() override;
            virtual void onSelectionChanged(std::optional<SelectionInfo>) override;

            void initShaderMacros();

            std::shared_ptr<IRenderingView> getCurrentRenderingView();

            void setupAllUniforms();

            virtual void makeCurrent();

        private:
            //! Each supported rendering view/scene, mapped to the valid viewing modes
            std::map<ViewingMode, std::shared_ptr<IRenderingView>> m_rendering_views;

            //! Whether this drawing area has been initialized
            bool m_initialized;
    };
}

#endif

